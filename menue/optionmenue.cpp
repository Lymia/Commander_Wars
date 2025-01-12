#include <QDir>
#include <QFileInfoList>
#include <QTextStream>
#include <QProcess>
#include <QScreen>
#include <QApplication>

#include "3rd_party/oxygine-framework/oxygine/actor/Stage.h"
#include "3rd_party/oxygine-framework/oxygine/res/SingleResAnim.h"

#include "menue/optionmenue.h"
#include "menue/mainwindow.h"

#include "coreengine/mainapp.h"
#include "coreengine/gameconsole.h"
#include "coreengine/settings.h"
#include "coreengine/audiomanager.h"

#include "resource_management/backgroundmanager.h"
#include "resource_management/objectmanager.h"
#include "resource_management/fontmanager.h"

#include "objects/base/label.h"
#include "objects/base/moveinbutton.h"
#include "objects/dialogs/dialogmessagebox.h"
#include "objects/dialogs/gamepadinfo.h"

#include "ui_reader/uifactory.h"

OptionMenue::OptionMenue(const QString & xmlFile)
    : m_xmlFile(xmlFile),
      m_gamemodeMods(
{
          // enabled                                // disabled
          OptionMenue::GamemodeMods(QStringList(), {"mods/aw_unloading",
                                                    "mods/aw2_damage_formula",
                                                    "mods/awds_unit",
                                                    "mods/awds_co",
                                                    "mods/awds_weather",
                                                    "mods/awdc_co",
                                                    "mods/awdc_unit",
                                                    "mods/awdc_powergain",
                                                    "mods/awdc_weather",
                                                    "mods/awdc_terrain",
                                                    "mods/awdc_flare",
                                                    "map_creator",
                                                    "coop_mod"}),
          OptionMenue::GamemodeMods(
              {"mods/aw_unloading",
               "mods/aw2_damage_formula",
               "mods/awds_unit",
               "mods/awds_co",
               "mods/awds_weather"},
              {"mods/awdc_co",
               "mods/awdc_unit",
               "mods/awdc_powergain",
               "mods/awdc_weather",
               "mods/awdc_terrain",
               "mods/awdc_flare",
               "map_creator",
               "coop_mod"}),
          OptionMenue::GamemodeMods(
              {"mods/aw_unloading",
               "mods/awdc_co",
               "mods/awdc_unit",
               "mods/awdc_powergain",
               "mods/awdc_weather",
               "mods/awdc_terrain",
               "mods/awdc_flare"},
              {"mods/aw2_damage_formula",
               "mods/awds_unit",
               "mods/awds_co",
               "mods/awds_weather",
               "map_creator",
               "coop_mod"})
          })
{
#ifdef GRAPHICSUPPORT
    setObjectName("OptionMenue");
#endif
    Mainapp* pApp = Mainapp::getInstance();
    pApp->pauseRendering();
    Interpreter::setCppOwnerShip(this);
    CONSOLE_PRINT("Entering Option Menue", GameConsole::eDEBUG);

    BackgroundManager* pBackgroundManager = BackgroundManager::getInstance();
    // load background
    oxygine::spSprite sprite = MemoryManagement::create<oxygine::Sprite>();
    addChild(sprite);
    oxygine::ResAnim* pBackground = pBackgroundManager->getResAnim("optionmenu");
    if (pBackground != nullptr)
    {
        sprite->setResAnim(pBackground);
        // background should be last to draw
        sprite->setPriority(static_cast<qint32>(Mainapp::ZOrder::Background));
        sprite->setScaleX(static_cast<float>(oxygine::Stage::getStage()->getWidth()) / static_cast<float>(pBackground->getWidth()));
        sprite->setScaleY(static_cast<float>(oxygine::Stage::getStage()->getHeight()) / static_cast<float>(pBackground->getHeight()));
    }

    pApp->getAudioManager()->clearPlayList();
    pApp->getAudioManager()->loadFolder("resources/music/credits_options");
    pApp->getAudioManager()->playRandom();

    oxygine::spButton pButtonExit = ObjectManager::createButton(tr("Exit"), 200);
    addChild(pButtonExit);
    pButtonExit->setPosition(oxygine::Stage::getStage()->getWidth()  / 2.0f + 10,
                             oxygine::Stage::getStage()->getHeight() - pButtonExit->getScaledHeight() - 10);
    pButtonExit->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
    {
        emit sigExitMenue();
    });
    connect(this, &OptionMenue::sigExitMenue, this, &OptionMenue::exitMenue, Qt::QueuedConnection);

    oxygine::spButton pButtonReset = ObjectManager::createButton(tr("Reset settings"), 200);
    addChild(pButtonReset);
    pButtonReset->setPosition(oxygine::Stage::getStage()->getWidth() / 2.0f - pButtonReset->getScaledWidth() - 10,
                             oxygine::Stage::getStage()->getHeight() - pButtonReset->getScaledHeight() - 10);
    pButtonReset->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
    {
        emit sigShowResetBox();
    });
    connect(this, &OptionMenue::sigShowResetBox, this, &OptionMenue::showResetBox, Qt::QueuedConnection);
    connect(this, &OptionMenue::sigUpdateModCheckboxes, this, &OptionMenue::updateModCheckboxes, Qt::QueuedConnection);
    connect(this, &OptionMenue::sigChangeScreenSize, pApp, &Mainapp::changeScreenSize, Qt::QueuedConnection);
    connect(this, &OptionMenue::sigReloadSettings, this, &OptionMenue::reloadSettings, Qt::QueuedConnection);
    connect(pApp, &Mainapp::sigWindowLayoutChanged, this, &OptionMenue::reloadSettings, Qt::QueuedConnection);

    if (xmlFile.isEmpty())
    {
        showMods();
    }
    else
    {
        UiFactory::getInstance().createUi(xmlFile, this);
    }
    pApp->continueRendering();
}

void OptionMenue::onEnter()
{
    Interpreter* pInterpreter = Interpreter::getInstance();
    QString object = "Init";
    QString func = "optionMenu";
    if (pInterpreter->exists(object, func))
    {
        CONSOLE_PRINT("Executing:" + object + "." + func, GameConsole::eDEBUG);
        QJSValueList args({pInterpreter->newQObject(this)});
        pInterpreter->doFunction(object, func, args);
    }
}

void OptionMenue::exitMenue()
{    
    // save changed settings :)
    Settings::getInstance()->saveSettings();
    if (m_restartNeeded)
    {
        spDialogMessageBox pMessage = MemoryManagement::create<DialogMessageBox>(tr("Some changes need a restart of the game. The game will restart. Press Ok to restart."), true);
        connect(pMessage.get(), &DialogMessageBox::sigOk, this, &OptionMenue::restart, Qt::QueuedConnection);
        addChild(pMessage);
    }
    else
    {
        CONSOLE_PRINT("Leaving Option Menue", GameConsole::eDEBUG);
        m_onEnterTimer.stop();
        auto window = MemoryManagement::create<Mainwindow>("ui/menu/mainoptionmenu.xml");
        oxygine::Stage::getStage()->addChild(window);
        oxygine::Actor::detach();
    }
}

void OptionMenue::reloadSettings()
{    
    CONSOLE_PRINT("Leaving Option Menue", GameConsole::eDEBUG);
    m_onEnterTimer.stop();
    spOptionMenue newMenu = MemoryManagement::create<OptionMenue>(m_xmlFile);
    // carry over restart flag
    newMenu->m_restartNeeded = m_restartNeeded;
    oxygine::Stage::getStage()->addChild(newMenu);
    oxygine::Actor::detach();
}

quint8 OptionMenue::getSupportedScreenCount()
{
    return QApplication::screens().size() - 1;
}

void OptionMenue::changeScreenSize(QSize size)
{
    Settings::getInstance()->setWidth(size.width());
    Settings::getInstance()->setHeight(size.height());
    emit sigChangeScreenSize(size.width(), size.height());
    emit sigReloadSettings();
}

void OptionMenue::loadModPanels()
{
    m_ModBoxes.clear();
    m_ModCheckboxes.clear();
    m_ModSelector = MemoryManagement::create<oxygine::Actor>();
    m_ModSelector->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    m_ModSelector->setPosition(10, 10);
    QSize size(oxygine::Stage::getStage()->getWidth() - 20,
               oxygine::Stage::getStage()->getHeight() - 70);
    size.setWidth(oxygine::Stage::getStage()->getWidth() - 60);
    size.setHeight(size.height() - 50);
    m_pMods = MemoryManagement::create<Panel>(true,  size - QSize(0, 50), size);
    m_pMods->setPosition(10, 110);
    m_ModSelector->addChild(m_pMods);
    size.setHeight(size.height() + 70);
    m_pModDescription = MemoryManagement::create<Panel>(true,  size, size, "panel");
    m_pModDescription->setPosition(oxygine::Stage::getStage()->getWidth() - 1, 25);
    auto moveInButton = MemoryManagement::create<MoveInButton>(m_pModDescription.get(), m_pModDescription->getScaledWidth() + 10);
    m_pModDescription->addChild(moveInButton);
    connect(this, &OptionMenue::sigUpdateModFilter, this, &OptionMenue::updateModFilter, Qt::QueuedConnection);
    connect(this, &OptionMenue::sigLoadModInfo, this, &OptionMenue::loadModInfo, Qt::QueuedConnection);
    connect(this, &OptionMenue::sigShowResetBox, this, &OptionMenue::showResetBox, Qt::QueuedConnection);
    addChild(m_ModSelector);
}

void OptionMenue::showMods()
{    
    Mainapp* pApp = Mainapp::getInstance();
    pApp->pauseRendering();
    loadModPanels();
    ObjectManager* pObjectManager = ObjectManager::getInstance();
    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getMainFont24());
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = true;
    m_ModDescriptionText = MemoryManagement::create<oxygine::TextField>();
    m_ModDescriptionText->setStyle(style);
    m_ModDescriptionText->setSize(m_pModDescription->getContentWidth() - 70, 500);
    m_ModDescriptionText->setX(10);
    m_modThumbnail = MemoryManagement::create<oxygine::Sprite>();
    m_modThumbnail->setPosition(10, 10);
    m_pModDescription->addItem(m_modThumbnail);
    m_pModDescription->addItem(m_ModDescriptionText);

    spLabel pLabel = MemoryManagement::create<Label>(260, true);
    style.multiline = false;
    pLabel->setStyle(style);
    pLabel->setHtmlText(tr("Advance Wars Game:"));
    m_ModSelector->addChild(pLabel);
    qint32 y = 0;
    QStringList versions = {tr("Unknown"),
                            tr("Commander Wars"),
                            tr("Advance Wars DS"),
                            tr("Advance Wars 4")};
    m_pModSelection = MemoryManagement::create<DropDownmenu>(300, versions);
    m_pModSelection->setTooltipText(tr("Select an Advance Wars Game preset to enable mods to mimic a specific Advance Wars Game."));
    m_pModSelection->setX(260);
    connect(m_pModSelection.get(), &DropDownmenu::sigItemChanged, this, &OptionMenue::selectMods, Qt::QueuedConnection);
    m_ModSelector->addChild(m_pModSelection);
    updateModSelection();
    y += pLabel->getHeight() + 10;
    pLabel = MemoryManagement::create<Label>(260, true);
    style.multiline = false;
    pLabel->setStyle(style);
    pLabel->setHtmlText(tr("Tag Filter:"));
    pLabel->setY(y);
    m_ModSelector->addChild(pLabel);
    QStringList tags;
    QStringList currentMods = Settings::getInstance()->getMods();
    qint32 width = 0;
    qint32 mods = 0;
    style.multiline = false;
    QStringList availableMods = Settings::getInstance()->getAvailableMods();
    for (const auto & mod : std::as_const(availableMods))
    {
        QString name;
        QString description;
        QString version;
        QStringList compatibleMods;
        QStringList incompatibleMods;
        QStringList requiredMods;
        bool isComsetic = false;
        QStringList modTags;
        QString thumbnail;
        Settings::getInstance()->getModInfos(mod, name, description, version,
                              compatibleMods, incompatibleMods, requiredMods, isComsetic,
                              modTags, thumbnail);
        for (const auto & tag : std::as_const(modTags))
        {
            if (!tags.contains(tag))
            {
                tags.append(tag);
            }
        }
        oxygine::ResAnim* pAnim = pObjectManager->getResAnim("topbar+dropdown");
        oxygine::spBox9Sprite pBox = MemoryManagement::create<oxygine::Box9Sprite>();
        pBox->setResAnim(pAnim);

        spCheckbox modCheck = MemoryManagement::create<Checkbox>();
        m_ModCheckboxes.append(modCheck);
        modCheck->setPosition(10, 5);
        pBox->addChild(modCheck);

        spLabel pTextfield = MemoryManagement::create<Label>(oxygine::Stage::getStage()->getWidth() - 190);
        pTextfield->setStyle(style);
        pTextfield->setHtmlText(name);
        pTextfield->setPosition(50, 5);
        pBox->addChild(pTextfield);
        if (currentMods.contains(mod))
        {
            modCheck->setChecked(true);
        }
        connect(modCheck.get(), &Checkbox::checkChanged, this, [this, mod](bool checked)
        {
            if (checked)
            {
                Settings::getInstance()->addMod(mod);
            }
            else
            {
                Settings::getInstance()->removeMod(mod);
            }
            m_restartNeeded = true;
            emit sigUpdateModCheckboxes();
        });
        pBox->setPosition(10, 10 + mods * 50);
        pBox->setSize(oxygine::Stage::getStage()->getWidth() - 130, 50);

        auto* pPtrBox = pBox.get();
        pBox->addClickListener([this, pPtrBox, name, description, version,
                               compatibleMods, incompatibleMods, requiredMods, isComsetic,
                               modTags, thumbnail](oxygine::Event* pEvent)
        {
            pEvent->stopPropagation();
            sigLoadModInfo(pPtrBox, name, description, version,
                           compatibleMods, incompatibleMods, requiredMods, isComsetic,
                           modTags, thumbnail);
        });
        m_ModBoxes.append(pBox);
        m_pMods->addItem(pBox);
        mods++;
    }
    m_pMods->setContentWidth(width);
    m_pMods->setContentHeigth(50 + mods * 50);

    tags.sort();
    tags.push_front(tr("All"));
    spDropDownmenu pTagSelection = MemoryManagement::create<DropDownmenu>(300, tags);
    pTagSelection->setTooltipText(tr("Filters mods by given tags."));
    pTagSelection->setPosition(260, y);
    connect(pTagSelection.get(), &DropDownmenu::sigItemChanged, this, [this, tags](qint32 value)
    {
        QString tag;
        if (value > 0)
        {
            tag = tags[value];
        }
        emit sigUpdateModFilter(tag);
    });
    m_ModSelector->addChild(pTagSelection);
    m_ModSelector->addChild(m_pModDescription);
    updateModFilter("");
    updateModCheckboxes();
    pApp->continueRendering();
}

void OptionMenue::updateModSelection()
{
    QStringList currentMods = Settings::getInstance()->getMods();
    qint32 index = 0;
    bool set = false;
    for (const auto & gameMode : std::as_const(m_gamemodeMods))
    {
        bool valid = (currentMods.size() == gameMode.m_enableMods.size());
        for (const auto & activeMod : gameMode.m_enableMods)
        {
            if (!currentMods.contains(activeMod))
            {
                valid = false;
            }
        }
        if (valid)
        {
            set = true;
            m_pModSelection->setCurrentItem(index + 1);
            break;
        }
        ++index;
    }
    if (!set)
    {
        m_pModSelection->setCurrentItem(0);
    }
}

void OptionMenue::loadModInfo(oxygine::Box9Sprite* pPtrBox,
                              QString name, QString description, QString version,
                              QStringList compatibleMods, QStringList incompatibleMods, QStringList requiredMods,
                              bool isComsetic, QStringList modTags, QString thumbnail)
{
    for (qint32 i2 = 0; i2 < m_ModBoxes.size(); i2++)
    {
        m_ModBoxes[i2]->addTween(oxygine::Sprite::TweenAddColor(QColor(0, 0, 0, 0)), oxygine::timeMS(300));
    }
    pPtrBox->addTween(oxygine::Sprite::TweenAddColor(QColor(32, 200, 32, 0)), oxygine::timeMS(300));
    qint32 y = 10;
    if (!thumbnail.isEmpty())
    {
        QImage img;
        if (QFile::exists(Settings::userPath() + thumbnail))
        {
            img = QImage(Settings::userPath() + thumbnail);
        }
        else if (QFile::exists(oxygine::Resource::RCC_PREFIX_PATH + thumbnail))
        {
            img = QImage(oxygine::Resource::RCC_PREFIX_PATH + thumbnail);
        }
        else
        {
            oxygine::handleErrorPolicy(oxygine::ep_show_error, "unable to locate thumbnail for mod " + name);
        }
        m_modThumbnail->setVisible(true);
        m_modThumbnailAnim = MemoryManagement::create<oxygine::SingleResAnim>();
        Mainapp::getInstance()->loadResAnim(m_modThumbnailAnim, img, 1, 1, 1.0f);
        m_modThumbnail->setResAnim(m_modThumbnailAnim.get());
        if (m_modThumbnailAnim->getWidth() > m_pModDescription->getContentWidth() - 60)
        {
            m_modThumbnail->setScale(static_cast<float>(m_pModDescription->getContentWidth() - 60) / static_cast<float>(m_modThumbnailAnim->getWidth()));
        }
        y += m_modThumbnail->getY() + m_modThumbnail->getScaledHeight() + 10;
    }
    else
    {
        m_modThumbnail->setVisible(false);
    }
    QString cosmeticInfo;
    if (isComsetic)
    {
        cosmeticInfo = QString("\n\n") + tr("The mod author designated this mod as 'Cosmetic' and may be used during multiplayer matches based on the game rules.");
    }
    QString modInfo = "\n\n" + tr("Compatible Mods:\n");
    for (const auto & mod : compatibleMods)
    {
        modInfo += Settings::getInstance()->getModName(mod) + "\n";
    }
    modInfo += "\n" + tr("Incompatible Mods:\n");
    for (const auto & mod : incompatibleMods)
    {
        modInfo += Settings::getInstance()->getModName(mod) + "\n";
    }
    modInfo += "\n" + tr("Required Mods:\n");
    for (const auto & mod : requiredMods)
    {
        modInfo += Settings::getInstance()->getModName(mod) + "\n";
    }
    modInfo += "\n" + tr("Tags:\n");
    for (const auto & tag : modTags)
    {
        modInfo += tag + "\n";
    }
    m_ModDescriptionText->setHtmlText(description + cosmeticInfo + modInfo + "\n\n" + tr("Version: ") + version);
    m_ModDescriptionText->setHeight(m_ModDescriptionText->getTextRect().height());
    m_ModDescriptionText->setY(y);
    m_pModDescription->setContentHeigth(y + m_ModDescriptionText->getTextRect().height() + 40);
}

void OptionMenue::selectMods(qint32 item)
{    
    if (item > 0)
    {
        QStringList removeList;
        QStringList addList;
        removeList = m_gamemodeMods[item - 1].m_disableMods;
        addList = m_gamemodeMods[item - 1].m_enableMods;
        for (auto & removeMod : removeList)
        {
            Settings::getInstance()->removeMod(removeMod);
        }
        for (auto & addMod : addList)
        {
            Settings::getInstance()->addMod(addMod);
        }
        CONSOLE_PRINT("Marking restart cause mods changed.", GameConsole::eDEBUG);
        m_restartNeeded = true;
        showMods();
    }
}

void OptionMenue::updateModCheckboxes()
{
    const auto availableMods = Settings::getInstance()->getAvailableMods();
    const auto mods = Settings::getInstance()->getActiveMods();
    for (auto & checkbox : m_ModCheckboxes)
    {
        checkbox->setEnabled(true);
    }
    for (const auto & mod : mods)
    {
        QString name;
        QString description;
        QString version;
        QStringList compatibleMods;
        QStringList incompatibleMods;
        QStringList requiredMods;
        QStringList tags;
        QString thumbnail;
        bool isComsetic = false;
        Settings::getInstance()->getModInfos(mod, name, description, version,
                              compatibleMods, incompatibleMods, requiredMods, isComsetic, tags, thumbnail);
        qint32 i2 = 0;
        for (const auto & checkBoxMod : std::as_const(availableMods))
        {
            if (incompatibleMods.contains(checkBoxMod))
            {
                m_ModCheckboxes[i2]->setEnabled(false);
            }
            ++i2;
        }
    }
    qint32 i = 0;
    for (const auto & mod : std::as_const(availableMods))
    {
        QString name;
        QString description;
        QString version;
        QStringList compatibleMods;
        QStringList incompatibleMods;
        QStringList requiredMods;
        QStringList tags;
        QString thumbnail;
        bool isComsetic = false;
        Settings::getInstance()->getModInfos(mod, name, description, version,
                              compatibleMods, incompatibleMods, requiredMods, isComsetic, tags, thumbnail);
        for (const auto & incompatibleMod : std::as_const(incompatibleMods))
        {
            if (mods.contains(incompatibleMod))
            {
                m_ModCheckboxes[i]->setEnabled(false);
                break;
            }
        }
        if (m_ModCheckboxes[i]->getEnabled())
        {
            for (const auto & requiredMod : std::as_const(requiredMods))
            {
                if (!mods.contains(requiredMod))
                {
                    if (m_ModCheckboxes[i]->getChecked())
                    {
                        m_ModCheckboxes[i]->setChecked(false);
                        Settings::getInstance()->removeMod(mod);
                    }
                    m_ModCheckboxes[i]->setEnabled(false);
                    break;
                }
            }
        }
        ++i;
    }
    updateModSelection();
}

void OptionMenue::updateModFilter(QString tag)
{
    const auto mods = Settings::getInstance()->getAvailableMods();
    qint32 visibleCounter = 0;
    for (qint32 i = 0; i < m_ModBoxes.size(); ++i)
    {
        QString name;
        QString description;
        QString version;
        QStringList compatibleMods;
        QStringList incompatibleMods;
        QStringList requiredMods;
        QStringList tags;
        QString thumbnail;
        bool isComsetic = false;
        Settings::getInstance()->getModInfos(mods[i], name, description, version,
                              compatibleMods, incompatibleMods, requiredMods, isComsetic, tags, thumbnail);


        if (tag.isEmpty() ||
            tags.contains(tag))
        {
            m_ModBoxes[i]->setY(visibleCounter * 50);
            m_ModBoxes[i]->setVisible(true);
            ++visibleCounter;
        }
        else
        {
            m_ModBoxes[i]->setVisible(false);
        }
    }
    m_pMods->setContentHeigth(50 + visibleCounter * 50);
}

void OptionMenue::restart()
{
    CONSOLE_PRINT("Forcing restart to reload required data changed in the options.", GameConsole::eDEBUG);
    removeChildren();
    detach();
    emit Mainapp::getInstance()->sigQuit(1);
}

void OptionMenue::showGamepadInfo()
{
    spGamepadInfo pGamepadInfo = MemoryManagement::create<GamepadInfo>();
    addChild(pGamepadInfo);
}

void OptionMenue::showResetBox()
{
    spDialogMessageBox pMessage = MemoryManagement::create<DialogMessageBox>(tr("This will reset most settings including mods and key bindings. Press Ok to reset the settings. This will force a restart upon leaving this menu."), true);
    connect(pMessage.get(), &DialogMessageBox::sigOk, this, &OptionMenue::onReset, Qt::QueuedConnection);
    addChild(pMessage);
}

void OptionMenue::onReset()
{
    Settings::getInstance()->resetSettings();
    m_restartNeeded = true;
    reloadSettings();
}

void OptionMenue::markRestartNeeded()
{
    m_restartNeeded = true;
}

void OptionMenue::changeGameScale(qreal gameScale)
{
    Settings::getInstance()->setGameScale(gameScale);
    Mainapp::getInstance()->initStage();
    reloadSettings();
}
