#include "3rd_party/oxygine-framework/oxygine/actor/Stage.h"

#include "objects/dialogs/folderdialog.h"
#include "objects/dialogs/dialogmessagebox.h"

#include "objects/base/label.h"

#include "coreengine/interpreter.h"
#include "coreengine/mainapp.h"
#include "coreengine/globalutils.h"

#include "resource_management/objectmanager.h"

const char* const ROOT = "::::";

FolderDialog::FolderDialog(QString startFolder)
{
#ifdef GRAPHICSUPPORT
    setObjectName("FolderDialog");
#endif
    Mainapp* pApp = Mainapp::getInstance();
    pApp->pauseRendering();
    Interpreter::setCppOwnerShip(this);
    ObjectManager* pObjectManager = ObjectManager::getInstance();
    oxygine::spBox9Sprite pSpriteBox = MemoryManagement::create<oxygine::Box9Sprite>();
    oxygine::ResAnim* pAnim = pObjectManager->getResAnim("filedialog");
    pSpriteBox->setResAnim(pAnim);
    pSpriteBox->setSize(oxygine::Stage::getStage()->getWidth(), oxygine::Stage::getStage()->getHeight());
    addChild(pSpriteBox);
    pSpriteBox->setPosition(0, 0);
    pSpriteBox->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    setPriority(static_cast<qint32>(Mainapp::ZOrder::Dialogs));

    // current folder
    m_CurrentFolder = MemoryManagement::create<Textbox>(oxygine::Stage::getStage()->getWidth() - 60);
    m_CurrentFolder->setPosition(30, 30);
    pSpriteBox->addChild(m_CurrentFolder);
    m_CurrentFolder->setCurrentText(startFolder);
    connect(m_CurrentFolder.get(), &Textbox::sigTextChanged, this, &FolderDialog::showFolder, Qt::QueuedConnection);
    // folder file selection
    m_MainPanel = MemoryManagement::create<Panel>(true, QSize(oxygine::Stage::getStage()->getWidth() - 60, oxygine::Stage::getStage()->getHeight() - 150), QSize(oxygine::Stage::getStage()->getWidth() - 60, oxygine::Stage::getStage()->getHeight() - 300));
    m_MainPanel->setPosition(30, 30 + m_CurrentFolder->getScaledHeight() + 10);
    pSpriteBox->addChild(m_MainPanel);

    // ok button
    m_OkButton = pObjectManager->createButton(tr("Ok"), 150);
    m_OkButton->setPosition(oxygine::Stage::getStage()->getWidth() - 30 - m_OkButton->getScaledWidth(),
                            oxygine::Stage::getStage()->getHeight() - 30 - m_OkButton->getScaledHeight());
    pSpriteBox->addChild(m_OkButton);
    auto* pCurrentFolder = m_CurrentFolder.get();
    m_OkButton->addEventListener(oxygine::TouchEvent::CLICK, [this, pCurrentFolder](oxygine::Event*)
    {
        QDir folder(pCurrentFolder->getCurrentText());
        QDir virtFolder(oxygine::Resources::RCC_PREFIX_PATH + pCurrentFolder->getCurrentText());
        if (folder.exists() || virtFolder.exists())
        {
            emit sigFolderSelected(pCurrentFolder->getCurrentText());
        }
        emit sigFinished();
    });
    // cancel button
    m_CancelButton = pObjectManager->createButton(tr("Cancel"), 150);
    m_CancelButton->setPosition(30,
                                oxygine::Stage::getStage()->getHeight() - 30 - m_CancelButton->getScaledHeight());
    pSpriteBox->addChild(m_CancelButton);
    m_CancelButton->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event*)
    {
        emit sigCancel();
    });
    connect(this, &FolderDialog::sigShowFolder, this, &FolderDialog::showFolder, Qt::QueuedConnection);
    showFolder(startFolder);
    connect(pApp, &Mainapp::sigKeyDown, this, &FolderDialog::KeyInput, Qt::QueuedConnection);
    connect(this, &FolderDialog::sigCancel, this, &FolderDialog::remove, Qt::QueuedConnection);
    connect(this, &FolderDialog::sigFinished, this, &FolderDialog::remove, Qt::QueuedConnection);
    pApp->continueRendering();
}

void FolderDialog::remove()
{
    detach();
}

void FolderDialog::showFolder(QString folder)
{
    Mainapp* pApp = Mainapp::getInstance();
    pApp->pauseRendering();
    // clean up current panel items
    for (qint32 i = 0; i < m_Items.size(); i++)
    {
        m_MainPanel->removeItem(m_Items[i]);
    }    
    CONSOLE_PRINT("Showing folder: " + folder, GameConsole::eDEBUG);
    folder = folder.replace("\\", "/");
    while (folder.contains("//"))
    {
        folder = folder.replace("//", "/");
    }
    folder = QDir(folder).absolutePath();
    folder = GlobalUtils::makePathRelative(folder);
    m_Items.clear();
    QDir dir(Settings::userPath() + folder);
    QDir virtDir(oxygine::Resource::RCC_PREFIX_PATH + folder);
    if (!dir.exists() && !virtDir.exists())
    {
        if (!folder.isEmpty())
        {
            CONSOLE_PRINT("Using root cause given folder wasn't found: " + folder, GameConsole::eINFO);
        }
        folder = ROOT;
    }

    // this is the we have to do all the work function of the file dialog...
    // we want the root folder
    QFileInfoList infoList;
    if (folder == ROOT)
    {
        infoList = QDir::drives();
    }
    else
    {
        infoList = GlobalUtils::getInfoList(folder);
    }

    qint32 itemCount = 0;
    for (qint32 i = 0; i < infoList.size(); i++)
    {
        QString myPath;
        if (folder == ROOT)
        {
            myPath = infoList[i].canonicalFilePath();
        }
        else if (infoList[i].canonicalFilePath() != QCoreApplication::applicationDirPath() &&
                 infoList[i].canonicalFilePath() != QCoreApplication::applicationDirPath() + "/")
        {
           myPath = GlobalUtils::makePathRelative(infoList[i].canonicalFilePath());
        }
        else
        {
            myPath = infoList[i].canonicalFilePath();
        }
        if (myPath == folder)
        {
            // skip ourself
            continue;
        }
        ObjectManager* pObjectManager = ObjectManager::getInstance();
        oxygine::ResAnim* pAnim = pObjectManager->getResAnim("filedialogitems");
        oxygine::spBox9Sprite pBox = MemoryManagement::create<oxygine::Box9Sprite>();
        pBox->setResAnim(pAnim);
        pBox->setSize(m_MainPanel->getScaledWidth() - 70, 40);
        pBox->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));

        spLabel textField = MemoryManagement::create<Label>(pBox->getScaledWidth() - 18);
        textField->setX(13);
        textField->setY(5);
        pBox->addChild(textField);
        m_MainPanel->addItem(pBox);
        // add some event handling :)
        auto* pPtrBox = pBox.get();
        pBox->addEventListener(oxygine::TouchEvent::OVER, [=](oxygine::Event*)
        {
            pPtrBox->addTween(oxygine::Sprite::TweenAddColor(QColor(32, 200, 32, 0)), oxygine::timeMS(300));
        });
        pBox->addEventListener(oxygine::TouchEvent::OUTX, [=](oxygine::Event*)
        {
            pPtrBox->addTween(oxygine::Sprite::TweenAddColor(QColor(0, 0, 0, 0)), oxygine::timeMS(300));
        });
        pBox->setPosition(0, itemCount * 40);

        // loop through all entries :)
        if (infoList[i].isDir())
        {
            if (folder == ROOT)
            {
                textField->setHtmlText(infoList[i].canonicalFilePath());
            }
            else
            {
                QString path = GlobalUtils::makePathRelative(infoList[i].filePath()).replace(folder, "");
                textField->setHtmlText(path);
            }
            pBox->addEventListener(oxygine::TouchEvent::CLICK, [this, myPath](oxygine::Event*)
            {
                emit sigShowFolder(myPath);
            });
        }
        else
        {
            // not possible i hope
        }
        m_Items.append(pBox);
        itemCount++;
    }
    m_MainPanel->setContentHeigth(itemCount * 40 + 50);
    if (folder == ROOT)
    {
        m_CurrentFolder->setCurrentText("");
    }
    else
    {
        m_CurrentFolder->setCurrentText(folder);
    }
    pApp->continueRendering();
}

void FolderDialog::deleteItem()
{
    QString folder = m_CurrentFolder->getCurrentText();
    QDir dir(folder);
    dir.removeRecursively();
    dir.cdUp();
    showFolder(dir.path());
    m_CurrentFolder->setCurrentText(dir.path());
    m_focused = true;
}

void FolderDialog::KeyInput(oxygine::KeyEvent event)
{
    // for debugging
    Qt::Key cur = event.getKey();
    if (!m_CurrentFolder->getFocused() &&
        m_focused)
    {
        switch(cur)
        {
            case Qt::Key_Delete:
            {
                if (QFile::exists(m_CurrentFolder->getCurrentText()))
                {
                    m_focused = false;
                    spDialogMessageBox pSurrender = MemoryManagement::create<DialogMessageBox>(tr("Do you want to delete the folder ") + m_CurrentFolder->getCurrentText() + "?", true);
                    connect(pSurrender.get(), &DialogMessageBox::sigOk, this, &FolderDialog::deleteItem, Qt::QueuedConnection);
                    connect(pSurrender.get(), &DialogMessageBox::sigCancel, this, [this]()
                    {
                        m_focused = true;
                    });
                    addChild(pSurrender);
                }
                break;
            }
            default:
            {
                break;
            }
        }
        
    }
}
