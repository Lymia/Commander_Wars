#include <QFile>
#ifdef GRAPHICSUPPORT
#include <QApplication>
#else
#include <QCoreApplication>
#endif
#include <QScreen>
#include <QDir>
#include <QMessageBox>
#include <QThread>
#include <QResource>
#include <QCoreApplication>
#include <QClipboard>

#include "coreengine/userdata.h"
#include "coreengine/mainapp.h"
#include "coreengine/interpreter.h"
#include "coreengine/audiomanager.h"
#include "coreengine/workerthread.h"
#include "coreengine/globalutils.h"
#include "coreengine/gameversion.h"

#include "ai/aiprocesspipe.h"

#include "objects/loadingscreen.h"

#include "network/tcpclient.h"
#include "network/mainserver.h"

#include "menue/basegamemenu.h"

#include "game/gamerecording/gamemapimagesaver.h"

#include "objects/minimap.h"

#include "resource_management/backgroundmanager.h"
#include "resource_management/buildingspritemanager.h"
#include "resource_management/cospritemanager.h"
#include "resource_management/fontmanager.h"
#include "resource_management/gameanimationmanager.h"
#include "resource_management/gamemanager.h"
#include "resource_management/gamerulemanager.h"
#include "resource_management/objectmanager.h"
#include "resource_management/terrainmanager.h"
#include "resource_management/unitspritemanager.h"
#include "resource_management/battleanimationmanager.h"
#include "resource_management/coperkmanager.h"
#include "resource_management/achievementmanager.h"
#include "resource_management/shoploader.h"
#include "resource_management/movementtablemanager.h"
#include "resource_management/weaponmanager.h"
#include "resource_management/movementplanneraddinmanager.h"
#include "resource_management/uimanager.h"

#include "wiki/wikidatabase.h"

#include "coreengine/crashreporter.h"

Mainapp* Mainapp::m_pMainapp{nullptr};

bool Mainapp::m_slave{false};
bool Mainapp::m_trainingSession{false};
const char* const Mainapp::GAME_CONTEXT = "GAME";

Mainapp::Mainapp()
    : m_aiProcessPipe(MemoryManagement::create<AiProcessPipe>())
{
#ifdef GRAPHICSUPPORT
    setObjectName("Mainapp");
#endif
    Interpreter::setCppOwnerShip(this);
    m_pMainapp = this;
    QThread::currentThread()->setObjectName("Renderthread");
    MemoryManagement::getInstance().moveToThread(QThread::currentThread());
    m_Workerthread = MemoryManagement::createNamedQObject<QThread>("QThread");
    m_Networkthread = MemoryManagement::createNamedQObject<QThread>("QThread");
    m_aiSubProcess = MemoryManagement::createNamedQObject<QProcess>("QProcess");
#ifdef AUDIOSUPPORT
    m_audioThread = MemoryManagement::createNamedQObject<QThread>("QThread");
    m_audioThread->setObjectName("Audiothread");
#endif
#ifdef GRAPHICSUPPORT
    m_Workerthread->setObjectName("Workerthread");
    m_Networkthread->setObjectName("Networkthread");
#endif
    m_Worker = MemoryManagement::create<WorkerThread>();
    connect(this, &Mainapp::sigShowCrashReport, this, &Mainapp::showCrashReport, Qt::QueuedConnection);
    connect(this, &Mainapp::sigChangePosition, this, &Mainapp::changePosition, Qt::QueuedConnection);
    connect(this, &Mainapp::activeChanged, this, &Mainapp::onActiveChanged, Qt::QueuedConnection);
    connect(this, &Mainapp::sigNextStartUpStep, this, &Mainapp::nextStartUpStep, Qt::QueuedConnection);
    connect(this, &Mainapp::sigCreateLineEdit, this, &Mainapp::createLineEdit, Qt::BlockingQueuedConnection);
    connect(this, &Mainapp::sigDoMapshot, this, &Mainapp::doMapshot, Qt::BlockingQueuedConnection);
    connect(this, &Mainapp::sigSaveMapAsImage, this, &Mainapp::saveMapAsImage, Qt::BlockingQueuedConnection);
    CrashReporter::setSignalHandler(&Mainapp::showCrashReport);
}

Mainapp::~Mainapp()
{
    CrashReporter::setSignalHandler(nullptr);
}

void Mainapp::createLineEdit()
{
#ifdef GRAPHICSUPPORT
    if (GameConsole::hasInstance())
    {
        CONSOLE_PRINT("Mainapp::createLineEdit", GameConsole::eDEBUG);
    }
    m_pLineEdit = MemoryManagement::create<EventTextEdit>();
    m_pLineEdit->setVisible(false);
#endif
}

void Mainapp::resetLineEdit()
{
#ifdef GRAPHICSUPPORT
    m_pLineEdit.reset();
#endif
}

void Mainapp::shutdown()
{
    m_aiSubProcess->kill();
    if (BuildingSpriteManager::created())
    {
        BuildingSpriteManager::getInstance()->free();
    }
    if (COSpriteManager::created())
    {
        COSpriteManager::getInstance()->free();
    }
    if (GameManager::created())
    {
        GameManager::getInstance()->free();
    }
    if (GameRuleManager::created())
    {
        GameRuleManager::getInstance()->free();
    }
    if (MovementTableManager::created())
    {
        MovementTableManager::getInstance()->free();
    }
    if (TerrainManager::created())
    {
        TerrainManager::getInstance()->free();
    }
    if (UnitSpriteManager::created())
    {
        UnitSpriteManager::getInstance()->free();
    }
    if (WeaponManager::created())
    {
        WeaponManager::getInstance()->free();
    }
    if (BattleAnimationManager::created())
    {
        BattleAnimationManager::getInstance()->free();
    }
    if (COPerkManager::created())
    {
        COPerkManager::getInstance()->free();
    }
    if (WikiDatabase::created())
    {
        WikiDatabase::getInstance()->free();
    }
    if (AchievementManager::created())
    {
        AchievementManager::getInstance()->free();
    }
    if (BackgroundManager::created())
    {
        BackgroundManager::getInstance()->free();
    }
    // FontManager::getInstance()->free();
    if (GameAnimationManager::created())
    {
        GameAnimationManager::getInstance()->free();
    }
    if (ObjectManager::created())
    {
        ObjectManager::getInstance()->free();
    }
    if (ShopLoader::created())
    {
        ShopLoader::getInstance()->free();
    }
    GameWindow::shutdown();
}

bool Mainapp::isWorker()
{
    return QThread::currentThread() == m_Workerthread.get() ||
            ((QThread::currentThread() == m_pMainThread || m_pMainThread == nullptr) &&
             (m_shuttingDown || !m_Worker->getStarted()));
}

bool Mainapp::isWorkerRunning()
{
    return m_Worker->getStarted();
}

void Mainapp::loadRessources()
{
    redrawUi();
    emit sigNextStartUpStep(StartupPhase::Start);
}

void Mainapp::nextStartUpStep(StartupPhase step)
{
    if (m_pMainThread == nullptr)
    {
        m_pMainThread = QThread::currentThread();
#ifdef GRAPHICSUPPORT
        m_pMainThread->setObjectName("Mainthread");
#endif
    }
#ifdef GRAPHICSUPPORT
    if (m_noUi)
    {
        GameConsole::setModuleMode(GameConsole::eResources, false);
    }
#else
    GameConsole::setModuleMode(GameConsole::eResources, false);
#endif
    GameConsole::print("Loading startup phase: " + QString::number(step), GameConsole::eDEBUG);
    spLoadingScreen pLoadingScreen = LoadingScreen::getInstance();
    m_startUpStep = step;
    bool automaticNextStep = true;
    switch (step)
    {
        case StartupPhase::General:
        {
            CONSOLE_PRINT("Launching game with version: " + GameVersion().toString(), GameConsole::eDEBUG);
            QStringList mods = Settings::getInstance()->getMods();
            for (const auto & mod : mods)
            {
                CONSOLE_PRINT("Launching game with mods: " + mod, GameConsole::eDEBUG);
            }
            m_aiProcessPipe->moveToThread(m_Workerthread.get());
            emit m_aiProcessPipe->sigStartPipe();
            pLoadingScreen->moveToThread(m_Workerthread.get());
            m_AudioManager = MemoryManagement::create<AudioManager>(m_noAudio);
            // start after ressource loading
#ifdef GRAPHICSUPPORT
            m_Networkthread->setObjectName("NetworkThread");
            m_Workerthread->setObjectName("WorkerThread");
#endif
#ifdef AUDIOSUPPORT
            m_audioThread->start(QThread::Priority::HighPriority);
            m_AudioManager->moveToThread(m_audioThread.get());
            m_AudioManager->initAudio();
            m_AudioManager->clearPlayList();
            m_AudioManager->loadFolder("resources/music/hauptmenue");
#endif
            FontManager::getInstance();
            // load ressources by creating the singletons
            BackgroundManager::getInstance();
            spLoadingScreen pLoadingScreen = LoadingScreen::getInstance();
            pLoadingScreen->show();

            pLoadingScreen->setProgress(tr("Checking for new version..."), step  * stepProgress);
            redrawUi();
            break;
        }
        case UpdateManager:
        {
#ifdef UPDATESUPPORT
            GameUpdater::cleanUpOldArtifacts();
            QString updateStep = Settings::getInstance()->getUpdateStep();
            if (!getSlave() && !Settings::getInstance()->getAiSlave())
            {
                if ((GameVersion().getSufix() != "dev" && Settings::getInstance()->getAutomaticUpdates()) ||
                    updateStep == GameUpdater::MODE_FORCE ||
                    updateStep == GameUpdater::MODE_INSTALL)
                {
                    automaticNextStep = false;
                    m_gameUpdater = MemoryManagement::create<GameUpdater>();
                }
            }
            break;
#endif
        }
        case StartupPhase::ObjectManager:
        {
#ifdef UPDATESUPPORT
            m_gameUpdater.reset();
#endif
            ObjectManager::getInstance();
            pLoadingScreen->setProgress(tr("Loading Building Textures ..."), step  * stepProgress);
            redrawUi();
            break;
        }
        case StartupPhase::Building:
        {
            if (m_AudioManager.get() != nullptr)
            {
                m_AudioManager->playRandom();
            }
            redrawUi();
            BuildingSpriteManager::getInstance();
            pLoadingScreen->setProgress(tr("Loading CO Textures..."), step  * stepProgress);
            break;
        }
        case StartupPhase::COSprites:
        {
            redrawUi();
            COSpriteManager::getInstance();
            pLoadingScreen->setProgress(tr("Loading Animation Textures..."), step  * stepProgress);
            break;
        }
        case StartupPhase::GameAnimations:
        {
            redrawUi();
            GameAnimationManager::getInstance();
            pLoadingScreen->setProgress(tr("Loading Game Textures ..."), step  * stepProgress);
            break;
        }
        case StartupPhase::GameManager:
        {
            redrawUi();
            GameManager::getInstance();
            pLoadingScreen->setProgress(tr("Loading Rule Textures ..."), step  * stepProgress);
            break;
        }
        case StartupPhase::GameRuleManager:
        {
            redrawUi();
            GameRuleManager::getInstance();
            WeaponManager::getInstance();
            MovementTableManager::getInstance();
            pLoadingScreen->setProgress(tr("Loading Terrain Textures ..."), step  * stepProgress);
            break;
        }
        case StartupPhase::TerrainManager:
        {
            redrawUi();
            TerrainManager::getInstance();
            pLoadingScreen->setProgress(tr("Loading Units Textures ..."), step  * stepProgress);
            break;
        }
        case StartupPhase::UnitSpriteManager:
        {
            redrawUi();
            UnitSpriteManager::getInstance();
            pLoadingScreen->setProgress(tr("Loading Battleanimation Textures ..."), step  * stepProgress);
            break;
        }
        case StartupPhase::BattleAnimationManager:
        {

            BattleAnimationManager::getInstance();
            pLoadingScreen->setProgress(tr("Loading CO-Perk Textures ..."), step  * stepProgress);
            break;
        }
        case StartupPhase::COPerkManager:
        {
            redrawUi();
            COPerkManager::getInstance();
            pLoadingScreen->setProgress(tr("Loading Wiki Textures ..."), step  * stepProgress);
            break;
        }
        case StartupPhase::WikiDatabase:
        {
            redrawUi();
            WikiDatabase::getInstance();
            pLoadingScreen->setProgress(tr("Loading Userdata ..."), step  * stepProgress);
            break;
        }
        case StartupPhase::Userdata:
        {
            redrawUi();
            Userdata::getInstance();
            pLoadingScreen->setProgress(tr("Loading Achievement Textures ..."), step  * stepProgress);
            break;
        }
        case StartupPhase::Achievementmanager:
        {
            redrawUi();
            AchievementManager::getInstance();
            pLoadingScreen->setProgress(tr("Loading Shop Textures ..."), step  * stepProgress);
            break;
        }
        case MovementPlannerAddInManager:
        {
            redrawUi();
            MovementPlannerAddInManager::getInstance();
            pLoadingScreen->setProgress(tr("Loading Movement planner addin Textures ..."), step  * stepProgress);
            break;
        }
        case UiManager:
        {
            redrawUi();
            UiManager::getInstance();
            pLoadingScreen->setProgress(tr("Loading Ui Textures ..."), step  * stepProgress);
            break;
        }
        case StartupPhase::ShopLoader:
        {
            redrawUi();
            ShopLoader::getInstance();
            pLoadingScreen->setProgress(tr("Loading sounds ..."), step  * stepProgress);
            break;
        }
        case StartupPhase::Sound:
        {
            redrawUi();
            if (!m_noAudio && m_AudioManager.get() != nullptr)
            {
                m_AudioManager->createSoundCache();
            }
            pLoadingScreen->setProgress(tr("Loading Scripts ..."), SCRIPT_PROCESS);
            break;
        }
        case StartupPhase::LoadingScripts:
        {
            m_Networkthread->start(QThread::Priority::NormalPriority);
            redrawUi();
            if (!m_noUi)
            {
                // refresh timer cycle before using it.
                Settings::getInstance()->setFramesPerSecond(Settings::getInstance()->getFramesPerSecond());
                m_timer.setInterval(m_timerCycle);
                m_timer.start();
            }
            GameConsole::getInstance()->moveToThread(Mainapp::getWorkerthread());
            m_workerLaunched = true;
            m_Workerthread->start(QThread::Priority::NormalPriority);
            if (m_Worker != nullptr)
            {
                emit m_Worker->sigStart();
            }
            break;
        }
        case StartupPhase::Finalizing:
        {
            CONSOLE_PRINT("Finalizing boot", GameConsole::eDEBUG);
            if (Settings::getInstance()->getAiSlave())
            {
                CONSOLE_PRINT("Running as ai slave", GameConsole::eDEBUG);
                Settings::getInstance()->setAutoSavingCycle(0);
                emit m_aiProcessPipe->sigPipeReady();
            }
            else
            {
                if (!m_slave)
                {
                    m_gamepad.init();
                }
                if (Settings::getInstance()->getSpawnAiProcess())
                {
                    const char* const prefix = "--";
                    const QString program = QCoreApplication::applicationFilePath();
                    QStringList args({QString(prefix) + CommandLineParser::ARG_NOUI, // comment out for debugging
                                      QString(prefix) + CommandLineParser::ARG_NOAUDIO,
                                      QString(prefix) + CommandLineParser::ARG_MODS,
                                      Settings::getInstance()->getConfigString(Settings::getInstance()->getActiveMods()),
                                      QString(prefix) + CommandLineParser::ARG_SPAWNAIPROCESS,
                                      "0",
                                      QString(prefix) + CommandLineParser::ARG_AISLAVE});
                    CONSOLE_PRINT("Launching ai subprocess: " + program + " " +  args.join(" "), GameConsole::eDEBUG);
                    m_aiSubProcess->setObjectName("AiSubprocess");
                    m_aiSubProcess->start(program, args);
                }               
                if (m_slave && m_initScript.isEmpty())
                {
                    emit m_Worker->sigStartSlaveGame();
                }
                else
                {
                    emit m_Worker->sigShowMainwindow();
                }
            }
            break;
        }
    }
    if (step < StartupPhase::LoadingScripts && automaticNextStep)
    {
        emit sigNextStartUpStep(static_cast<StartupPhase>(static_cast<qint8>(step) + 1));
    }
}

void Mainapp::doScreenshot()
{
#ifdef GRAPHICSUPPORT
    auto* currentScreen = screen();
    if (currentScreen != nullptr)
    {
        auto picture = currentScreen->grabWindow(winId());
        qint32 i = 0;
        QDir dir("screenshots/");
        dir.mkpath(".");
        while (i < std::numeric_limits<qint32>::max())
        {
            QString filename = "screenshots/screenshot+" + QString::number(i) + ".png";
            if (!QFile::exists(filename))
            {
                picture.save(filename);
                break;
            }
            ++i;
        }
    }
#endif
}

void Mainapp::changeScreen(quint8 screen)
{
#ifdef GRAPHICSUPPORT
    if (m_noUi)
    {
        return;
    }
    CONSOLE_PRINT("Changing assigned screen to " + QString::number(screen), GameConsole::eDEBUG);
    auto screens = QApplication::screens();
    if (screen >= screens.size())
    {
        screen = 0;
        Settings::getInstance()->setScreen(screen);
    }
    setScreen(screens[screen]);
#endif
}

void Mainapp::changeScreenMode(Settings::ScreenModes mode)
{
#ifdef GRAPHICSUPPORT
    if (m_noUi)
    {
        return;
    }
    changeScreen(Settings::getInstance()->getScreen());
    CONSOLE_PRINT("Changing screen mode to " + QString::number(static_cast<qint32>(mode)), GameConsole::eDEBUG);
    hide();
    auto screens = QApplication::screens();
    switch (mode)
    {
        case Settings::ScreenModes::Borderless:
        {
            setWindowState(Qt::WindowState::WindowNoState);
            setFlag(Qt::FramelessWindowHint);
            QScreen* screen = screens[Settings::getInstance()->getScreen()];
            QRect screenSize = screen->availableGeometry();
            setPosition(screenSize.x(), screenSize.y());
            Settings::getInstance()->setFullscreen(false);
            Settings::getInstance()->setBorderless(true);
            setWidth(screenSize.width());
            Settings::getInstance()->setWidth(screenSize.width());
            setHeight(screenSize.height());
            Settings::getInstance()->setHeight(screenSize.height());
            show();
            break;
        }
        case Settings::ScreenModes::FullScreen:
        {
            Settings::getInstance()->setFullscreen(true);
            Settings::getInstance()->setBorderless(false);
#ifdef ANDROID
            showMaximized();
            // set window info
            Settings::getInstance()->setWidth(width());
            Settings::getInstance()->setHeight(height());
#else
            QScreen* screen = screens[Settings::getInstance()->getScreen()];
            QRect screenSize = screen->geometry();
            // set window info
            Settings::getInstance()->setWidth(screenSize.width());
            Settings::getInstance()->setHeight(screenSize.height());
            setPosition(screenSize.x(), screenSize.y());
            setGeometry(screenSize);
            showFullScreen();
#endif
            break;
        }
        default:
        {
            setWindowState(Qt::WindowState::WindowNoState);
            setFlag(Qt::FramelessWindowHint, false);
            Settings::getInstance()->setFullscreen(false);
            Settings::getInstance()->setBorderless(false);
            QScreen* screen = QApplication::primaryScreen();
            QRect screenSize = screen->availableGeometry();
            if (screenSize.width() < Settings::getInstance()->getWidth())
            {
                setWidth(screenSize.width());
                Settings::getInstance()->setWidth(screenSize.width());
            }
            if (screenSize.height() < Settings::getInstance()->getHeight())
            {
                setHeight(screenSize.height());
                Settings::getInstance()->setHeight(screenSize.height());
            }
            showNormal();
        }
    }
    // change screen size after changing the border flags
    changeScreenSize(Settings::getInstance()->getWidth(), Settings::getInstance()->getHeight());
    // ensure brightness and gamma are correct
    setBrightness(Settings::getInstance()->getBrightness());
    setGamma(Settings::getInstance()->getGamma());
#endif
}

void Mainapp::changeScreenSize(qint32 width, qint32 heigth)
{    
#ifdef GRAPHICSUPPORT
    if (m_noUi)
    {
        return;
    }
    CONSOLE_PRINT("Changing screen size to width: " + QString::number(width) + " height: " + QString::number(heigth), GameConsole::eDEBUG);
    resize(width, heigth);
    setMinimumSize(QSize(width, heigth));
    setMaximumSize(QSize(width, heigth));

    Settings::getInstance()->setWidth(width);
    Settings::getInstance()->setHeight(heigth);
    Settings::getInstance()->saveSettings();
    initStage();
    emit sigWindowLayoutChanged();
    emit sigChangePosition(QPoint(-1, -1), true);
#endif
}

QPoint Mainapp::mapPosFromGlobal(QPoint pos) const
{
    return mapFromGlobal(pos);
}

QPoint Mainapp::mapPosToGlobal(QPoint pos) const
{
    return mapToGlobal(pos);
}

void Mainapp::changePosition(QPoint pos, bool invert)
{
    if (m_noUi)
    {
        return;
    }
    setPosition(position() + pos);
    if (invert)
    {
        emit sigChangePosition(-pos, false);
    }
}

Settings::ScreenModes Mainapp::getScreenMode()
{
    if (Settings::getInstance()->getFullscreen())
    {
        return Settings::ScreenModes::FullScreen;
    }
    else if (Settings::getInstance()->getBorderless())
    {
        return Settings::ScreenModes::Borderless;
    }
    else
    {
        return Settings::ScreenModes::Window;
    }
}

void Mainapp::keyPressEvent(QKeyEvent *event)
{
    if (m_startUpStep >= StartupPhase::Finalizing)
    {
        Qt::Key cur = static_cast<Qt::Key>(event->key());
        if (cur == Settings::getInstance()->getKeyConsole())
        {
            emit GameConsole::getInstance()->sigToggleView();
        }
        else if (cur == Settings::getInstance()->getKey_screenshot())
        {
            doScreenshot();
        }
        else
        {
            emit sigKeyDown(oxygine::KeyEvent(event));
        }
    }
}

void Mainapp::keyReleaseEvent(QKeyEvent *event)
{
    if (m_startUpStep >= StartupPhase::Finalizing)
    {
        if (!event->isAutoRepeat())
        {
            emit sigKeyUp(oxygine::KeyEvent(event));
        }
    }
}

bool Mainapp::event(QEvent *event)
{
    bool handled = false;
    if (!m_shuttingDown)
    {
        FocusableObject* pObj(FocusableObject::getFocusedObject());
        if (pObj != nullptr)
        {
            handled = FocusableObject::handleEvent(event);
        }
        if (!handled)
        {
            if (event->type() == QEvent::InputMethod)
            {
#ifdef GRAPHICSUPPORT
                QInputMethodEvent* inputEvent = static_cast<QInputMethodEvent*>(event);
                handled = keyInputMethodEvent(inputEvent);
#else
                    handled = oxygine::GameWindow::event(event);
#endif
            }
            else
            {
                handled = oxygine::GameWindow::event(event);
            }
        }
    }
    return handled;
}

bool Mainapp::keyInputMethodEvent(QInputMethodEvent *event)
{
    emit sigKeyDown(oxygine::KeyEvent(event));
    return true;
}

bool Mainapp::getNoUi() const
{
    return m_noUi;
}

bool Mainapp::getSlave()
{
    return m_slave;
}

void Mainapp::setSlave(bool slave)
{
    m_slave = slave;
}

void Mainapp::showCrashReport(const QString & log)
{
    static qint32 counter = 0;
    if (QCoreApplication::instance()->thread() == QThread::currentThread())
    {
        // gui thread cool show the crash report
        QString title = tr("Whoops Sturm crashed a meteor into the PC.");
        // QMessageBox::critical(nullptr, title, log);
        QMessageBox criticalBox;
        criticalBox.setIcon(QMessageBox::Critical);
        criticalBox.setWindowTitle(title);
        criticalBox.setTextFormat(Qt::RichText);
        criticalBox.setText(tr("Please use the details or the crashlog to report a bug at \n<a href='https://github.com/Robosturm/Commander_Wars/issues'>https://github.com/Robosturm/Commander_Wars/issues</a>\n The game will be terminated sadly. :("));
        criticalBox.setDetailedText(log);
        criticalBox.exec();

        if (counter > 0)
        {
            // unlock crashed process
            counter--;
            m_pMainapp->m_crashMutex.unlock();
        }
    }
    else
    {
        // swap to gui thread
        counter++;
        m_pMainapp->m_crashMutex.lock();
        emit getInstance()->sigShowCrashReport(log);
        // lock crash thread
        m_pMainapp->m_crashMutex.lock();
        m_pMainapp->m_crashMutex.unlock();
    }
}

void Mainapp::setNoUi()
{
    m_noUi = true;
    m_timer.stop();
}

void Mainapp::setNoAudio()
{
    m_noAudio = true;
}

void Mainapp::actAsSlave()
{
    setSlave(true);
    Settings::getInstance()->setServer(false);
    Settings::getInstance()->setUsername("Server");
    m_slaveClient = MemoryManagement::create<TCPClient>(nullptr);
    m_slaveClient->moveToThread(getInstance()->getNetworkThread());
    CONSOLE_PRINT("Running as slave with name : " + Settings::getInstance()->getSlaveServerName(), GameConsole::eDEBUG);
}

void Mainapp::setSlaveClient(spNetworkInterface & client)
{
    m_slaveClient = client;
}

void Mainapp::onActiveChanged()
{
    if (!isActive())
    {
        FocusableObject::looseFocus();
    }
    if (m_AudioManager.get() != nullptr)
    {
        emit m_AudioManager->sigSetMuteInternal(!isActive());
    }
}

QString Mainapp::qsTr(QString text)
{
    return Mainapp::qsTr(text.toStdString().c_str());
}

QString Mainapp::qsTr(const char* const text)
{
    return QCoreApplication::translate(GAME_CONTEXT, text);
}

void Mainapp::createBaseDirs()
{
    QString userPath = Settings::getInstance()->getUserPath();
    CONSOLE_PRINT("Creating base dirs in " + userPath, GameConsole::eDEBUG);
    if (!userPath.isEmpty())
    {
        QDir newDir(userPath);
        newDir.mkpath(".");
    }
    QDir dir(userPath + "temp/");
    dir.removeRecursively();
    QStringList dirs =
    {
        "temp",
        "savegames",
        "mods",
        "data/gamerules",
        "data/randommaps",
        "data/records",
        "data/customStyles",
        "server/runningAutoMatches",
        "server/preparingAutoMatches",
        "maps",
        "customTerrainImages",
        "resources",
        "resources/aidata",
        "resources/aidata/very_easy",
        "resources/aidata/normal",
        "resources/aidata/heavy",
    };
    for (const auto & path : std::as_const(dirs))
    {
        QDir newDir(userPath + path);
        newDir.mkpath(".");
    }
    auto virtList = QDir(QString(oxygine::Resource::RCC_PREFIX_PATH) + "maps").entryInfoList(QDir::Dirs);
    for (const auto & item : std::as_const(virtList))
    {
        QString path = GlobalUtils::makePathRelative(item.canonicalFilePath());
        if (!path.endsWith(".camp"))
        {
            QDir newDir(userPath + path);
            newDir.mkpath(".");
        }
    }
}

void Mainapp::onQuit()
{
    const qint64 waitTime = 120;
    QCoreApplication::processEvents(QEventLoop::ProcessEventsFlag::AllEvents, 5);
    if (m_Worker != nullptr)
    {

        m_Worker = nullptr;
    }
    if (m_Workerthread->isRunning())
    {
        auto curTimte = QDateTime::currentSecsSinceEpoch();
        m_Workerthread->quit();
        while (!m_Workerthread->wait(1) &&
               QDateTime::currentSecsSinceEpoch() - curTimte < waitTime)
        {
            QCoreApplication::processEvents(QEventLoop::ProcessEventsFlag::AllEvents, 5);
        }
    }
    QCoreApplication::processEvents(QEventLoop::ProcessEventsFlag::AllEvents, 5);
    m_aiProcessPipe.reset();
#ifdef AUDIOSUPPORT
    if (m_AudioManager.get() != nullptr)
    {
        m_AudioManager->stopAudio();
        m_AudioManager.reset();
    }
    if (m_audioThread->isRunning())
    {
        auto curTimte = QDateTime::currentSecsSinceEpoch();
        m_audioThread->quit();
        while (!m_audioThread->wait(1) &&
               QDateTime::currentSecsSinceEpoch() - curTimte < waitTime)
        {
            QCoreApplication::processEvents(QEventLoop::ProcessEventsFlag::AllEvents, 5);
        }
    }
#endif
    QCoreApplication::processEvents(QEventLoop::ProcessEventsFlag::AllEvents, 5);
    if (m_Networkthread->isRunning())
    {
        m_Networkthread->quit();
        auto curTimte = QDateTime::currentSecsSinceEpoch();
        while (!m_Networkthread->wait(1) &&
               QDateTime::currentSecsSinceEpoch() - curTimte < waitTime)
        {
            QCoreApplication::processEvents(QEventLoop::ProcessEventsFlag::AllEvents, 5);
        }
    }
    QCoreApplication::processEvents(QEventLoop::ProcessEventsFlag::AllEvents, 5);
}

bool Mainapp::getTrainingSession()
{
    return m_trainingSession;
}

void Mainapp::setTrainingSession(bool newTrainingSession)
{
    m_trainingSession = newTrainingSession;
}

AiProcessPipe & Mainapp::getAiProcessPipe()
{
    return *(getInstance()->m_aiProcessPipe.get());
}

Mainapp::StartupPhase Mainapp::getStartUpStep() const
{
    return m_startUpStep;
}

void Mainapp::setInitScript(const QString &newInitScript)
{
    m_initScript = newInitScript;
}

spNetworkInterface Mainapp::getSlaveClient()
{
    return m_slaveClient;
}

const QString &Mainapp::getInitScript() const
{
    return m_initScript;
}

bool Mainapp::getCreateSlaveLogs() const
{
    return m_createSlaveLogs;
}

void Mainapp::setCreateSlaveLogs(bool create)
{
    m_createSlaveLogs = create;
}

void Mainapp::inputMethodQuery(Qt::InputMethodQuery query, QVariant arg)
{
    FocusableObject::handleInputMethodQuery(query, arg);
}

void Mainapp::doMapshot(BaseGamemenu* pMenu)
{
    if (beginRendering())
    {
        qint32 i = 0;
        QDir dir("screenshots/");
        dir.mkpath(".");
        while (i < std::numeric_limits<qint32>::max())
        {
            QString filename = "screenshots/mapshot+" + QString::number(i) + ".png";
            if (!QFile::exists(filename))
            {
                GamemapImageSaver::saveMapAsImage(filename, *pMenu);
                break;
            }
            ++i;
        }
    }
}

void Mainapp::saveMapAsImage(Minimap* pMinimap, QImage * img)
{
    if (!m_shuttingDown && !m_noUi)
    {
        if (isMainThread())
        {
            GamemapImageSaver::saveMapAsImage(pMinimap, *img);
        }
        else
        {
            emit sigSaveMapAsImage(pMinimap, img);
        }
    }
}
