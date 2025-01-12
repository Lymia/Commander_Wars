#include "3rd_party/oxygine-framework/oxygine/actor/Stage.h"

#include "objects/dialogs/rules/ruleselectiondialog.h"

#include "coreengine/mainapp.h"

#include "resource_management/objectmanager.h"

#include "game/gamemap.h"

#include "objects/dialogs/filedialog.h"

RuleSelectionDialog::RuleSelectionDialog(GameMap* pMap, RuleSelection::Mode mode, bool enabled)
    : m_pMap(pMap)
{
#ifdef GRAPHICSUPPORT
    setObjectName("RuleSelectionDialog");
#endif
    Interpreter::setCppOwnerShip(this);
    ObjectManager* pObjectManager = ObjectManager::getInstance();
    oxygine::spBox9Sprite pSpriteBox = MemoryManagement::create<oxygine::Box9Sprite>();
    oxygine::ResAnim* pAnim = pObjectManager->getResAnim("codialog");
    pSpriteBox->setResAnim(pAnim);
    pSpriteBox->setSize(oxygine::Stage::getStage()->getWidth(), oxygine::Stage::getStage()->getHeight());
    addChild(pSpriteBox);
    pSpriteBox->setPosition(0, 0);
    pSpriteBox->setPriority(static_cast<qint32>(Mainapp::ZOrder::Objects));
    setPriority(static_cast<qint32>(Mainapp::ZOrder::Dialogs));

    // ok button
    m_OkButton = pObjectManager->createButton(tr("Ok"), 150);
    m_OkButton->setPosition(oxygine::Stage::getStage()->getWidth() / 2 - m_OkButton->getScaledWidth() / 2,
                            oxygine::Stage::getStage()->getHeight() - 30 - m_OkButton->getScaledHeight());
    pSpriteBox->addChild(m_OkButton);
    m_OkButton->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event*)
    {
        emit sigOk();
    });
    connect(this, &RuleSelectionDialog::sigOk, this, &RuleSelectionDialog::pressedOk, Qt::QueuedConnection);

    if (enabled)
    {
        m_pButtonLoadRules = ObjectManager::createButton(tr("Load"));
        m_pButtonLoadRules->setPosition(oxygine::Stage::getStage()->getWidth() / 2 + 20 + m_OkButton->getScaledWidth() / 2, oxygine::Stage::getStage()->getHeight() - 30 - m_OkButton->getScaledHeight());
        m_pButtonLoadRules->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
        {
            emit sigShowLoadRules();
        });
        pSpriteBox->addChild(m_pButtonLoadRules);
        connect(this, &RuleSelectionDialog::sigShowLoadRules, this, &RuleSelectionDialog::showLoadRules, Qt::QueuedConnection);

        m_pButtonSaveRules = ObjectManager::createButton(tr("Save"));
        m_pButtonSaveRules->setPosition(oxygine::Stage::getStage()->getWidth() / 2 - m_pButtonSaveRules->getScaledWidth() - 20 - m_OkButton->getScaledWidth() / 2,
                                        oxygine::Stage::getStage()->getHeight() - 30 - m_OkButton->getScaledHeight());
        m_pButtonSaveRules->addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event * )->void
        {
            emit sigShowSaveRules();
        });
        pSpriteBox->addChild(m_pButtonSaveRules);
        connect(this, &RuleSelectionDialog::sigShowSaveRules, this, &RuleSelectionDialog::showSaveRules, Qt::QueuedConnection);
    }
    m_pRuleSelection = MemoryManagement::create<RuleSelection>(m_pMap, oxygine::Stage::getStage()->getWidth() - 80, mode, enabled);
    connect(m_pRuleSelection.get(), &RuleSelection::sigSizeChanged, this, &RuleSelectionDialog::ruleSelectionSizeChanged, Qt::QueuedConnection);
    QSize size(oxygine::Stage::getStage()->getWidth() - 20,
               oxygine::Stage::getStage()->getHeight() - 40 * 2 - m_OkButton->getScaledHeight());
    m_pPanel = MemoryManagement::create<Panel>(true,  size, size);
    m_pPanel->setPosition(10, 20);
    m_pPanel->addItem(m_pRuleSelection);
    m_pPanel->setContentHeigth(m_pRuleSelection->getScaledHeight() + 60);
    m_pPanel->setContentWidth(m_pRuleSelection->getScaledWidth() + 60);
    pSpriteBox->addChild(m_pPanel);
}

void RuleSelectionDialog::ruleSelectionSizeChanged()
{
    m_pPanel->setContentHeigth(m_pRuleSelection->getScaledHeight() + 60);
    m_pPanel->setContentWidth(m_pRuleSelection->getScaledWidth() + 60);
}

void RuleSelectionDialog::showLoadRules()
{    
    QStringList wildcards;
    wildcards.append("*.grl");
    QString path = Settings::userPath() + "data/gamerules";
    spFileDialog fileDialog = MemoryManagement::create<FileDialog>(path, wildcards, false, "", false, tr("Load"));
    addChild(fileDialog);
    connect(fileDialog.get(),  &FileDialog::sigFileSelected, this, &RuleSelectionDialog::loadRules, Qt::QueuedConnection);
}

void RuleSelectionDialog::showSaveRules()
{
    QStringList wildcards;
    wildcards.append("*.grl");
    QString path = Settings::userPath() + "data/gamerules";
    spFileDialog fileDialog = MemoryManagement::create<FileDialog>(path, wildcards, true, "", false, tr("Save"));
    addChild(fileDialog);
    connect(fileDialog.get(),  &FileDialog::sigFileSelected, this, &RuleSelectionDialog::saveRules, Qt::QueuedConnection);
}

void RuleSelectionDialog::loadRules(QString filename)
{
    if (filename.endsWith(".grl"))
    {
        QFile file(filename);
        if (file.exists())
        {
            QFile file(filename);
            file.open(QIODevice::ReadOnly);
            QDataStream stream(&file);
            stream.setVersion(QDataStream::Version::Qt_6_5);
            m_pMap->getGameRules()->deserializeObject(stream);
            file.close();
            auto mode = m_pRuleSelection->getMode();
            m_pRuleSelection->detach();
            m_pRuleSelection = MemoryManagement::create<RuleSelection>(m_pMap, oxygine::Stage::getStage()->getWidth() - 80, mode);
            m_pPanel->addItem(m_pRuleSelection);
            m_pPanel->setContentHeigth(m_pRuleSelection->getScaledHeight() + 40);
            m_pPanel->setContentWidth(m_pRuleSelection->getScaledWidth());
        }
    }
}

void RuleSelectionDialog::saveRules(QString filename)
{
    if (filename.endsWith(".grl"))
    {
        QFile file(filename);
        file.open(QIODevice::WriteOnly | QIODevice::Truncate);
        QDataStream stream(&file);
        stream.setVersion(QDataStream::Version::Qt_6_5);
        
        m_pMap->getGameRules()->serializeObject(stream);
        file.close();
    }    
}

void RuleSelectionDialog::pressedOk()
{
    emit sigRulesChanged();
    detach();    
}
