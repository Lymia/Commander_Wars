#include <QFile>

#include "3rd_party/oxygine-framework/oxygine/actor/Stage.h"
#include "3rd_party/oxygine-framework/oxygine/tween/TweenAnimColumn.h"
#include "3rd_party/oxygine-framework/oxygine/actor/ClipRectActor.h"

#include "game/gameanimation/gameanimationdialog.h"
#include "game/gameanimation/gameanimationfactory.h"
#include "game/gamemap.h"

#include "coreengine/interpreter.h"
#include "coreengine/audiomanager.h"

#include "resource_management/gamemanager.h"
#include "resource_management/fontmanager.h"
#include "resource_management/cospritemanager.h"

#include "menue/basegamemenu.h"

GameAnimationDialog::GameAnimationDialog(quint32 frameTime, GameMap* pMap)
    : GameAnimation (frameTime, pMap),
      m_finishTimer(this),
      m_textSpeed(100 / Settings::getInstance()->getDialogAnimationSpeed())
{
#ifdef GRAPHICSUPPORT
    setObjectName("GameAnimationDialog");
#endif
    Interpreter::setCppOwnerShip(this);
    connect(this, &GameAnimationDialog::sigStartFinishTimer, this, &GameAnimationDialog::startFinishTimer, Qt::QueuedConnection);
    connect(&m_finishTimer, &QTimer::timeout, this, [this]()
    {
        emitFinished();
    });
    m_BackgroundSprite = MemoryManagement::create<oxygine::Sprite>();
    m_BackgroundSprite->setPriority(-1);
    addChild(m_BackgroundSprite);

    GameManager* pGameManager = GameManager::getInstance();
    oxygine::ResAnim* pAnim = pGameManager->getResAnim("dialogfield+mask");
    m_TextMask = MemoryManagement::create<oxygine::Sprite>();
    if (pAnim != nullptr)
    {
        m_TextMask->setScaleX(static_cast<float>(oxygine::Stage::getStage()->getWidth()) / static_cast<float>(pAnim->getWidth()));
    }
    m_TextMask->setResAnim(pAnim);
    addChild(m_TextMask);

    pAnim = pGameManager->getResAnim("dialogfield");
    m_TextBackground = MemoryManagement::create<oxygine::Sprite>();
    if (pAnim != nullptr)
    {
        m_TextBackground->setScaleX(static_cast<float>(oxygine::Stage::getStage()->getWidth()) / static_cast<float>(pAnim->getWidth()));
    }
    m_TextBackground->setResAnim(pAnim);
    m_TextBackground->setPriority(1);
    addChild(m_TextBackground);

    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getFont("dialog48"));
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = true;

    oxygine::spClipRectActor pRect = MemoryManagement::create<oxygine::ClipRectActor>();
    pRect->setPosition(48 * 2 + 5, 6);
    pRect->setSize(oxygine::Stage::getStage()->getWidth() - pRect->getX() - 5, 96);

    m_TextField = MemoryManagement::create<oxygine::TextField>();
    m_TextField->setPosition(0, 0);
    m_TextField->setSize(pRect->getScaledWidth() - 5,
                         pRect->getScaledHeight());
    m_TextField->setStyle(style);
    pRect->addChild(m_TextField);
    pRect->setPriority(1);
    addChild(pRect);

    m_COSprite = MemoryManagement::create<oxygine::Sprite>();
    m_COSprite->setScale(2.0f);
    m_COSprite->setY(6);
    m_COSprite->setPriority(1);
    addChild(m_COSprite);

    setPositionTop(false);
    addEventListener(oxygine::TouchEvent::CLICK, [this](oxygine::Event *pEvent )->void
    {
        oxygine::TouchEvent* pTouchEvent = oxygine::safeCast<oxygine::TouchEvent*>(pEvent);
        if (pTouchEvent != nullptr)
        {
            if (pTouchEvent->mouseButton == oxygine::MouseButton::MouseButton_Right)
            {
                emit sigRightClick();
            }
            else if (pTouchEvent->mouseButton == oxygine::MouseButton::MouseButton_Left)
            {
                emit sigLeftClick();
            }
            pTouchEvent->stopPropagation();
        }
    });
    connect(this, &GameAnimationDialog::sigRightClick, GameAnimationFactory::getInstance(), &GameAnimationFactory::finishAllAnimationsWithEmitFinished, Qt::QueuedConnection);
    connect(this, &GameAnimationDialog::sigLeftClick, this, &GameAnimationDialog::nextDialogStep, Qt::QueuedConnection);
    connect(Mainapp::getInstance(), &Mainapp::sigKeyDown, this, &GameAnimationDialog::keyInput, Qt::QueuedConnection);
}

void GameAnimationDialog::keyInput(oxygine::KeyEvent event)
{
    if (!event.getContinousPress())
    {
        if (!m_stopped && m_writePosition > 0)
        {
            // for debugging
            Qt::Key cur = event.getKey();
            if (cur == Settings::getInstance()->getKey_confirm() ||
                cur == Settings::getInstance()->getKey_confirm2())
            {
                nextDialogStep();
            }
        }
    }
}

void GameAnimationDialog::nextDialogStep()
{
    if (!m_stopped && m_started)
    {
        if (m_paused)
        {
            m_paused = false;
            m_writePosition += 1;
            m_TextField->setHtmlText(m_Text.mid(0, m_writePosition));
        }
        else
        {
            if (m_writePosition >= m_Text.size())
            {
                onFinished(false);
            }
            else
            {
                float textHeight = m_TextField->getTextRect().height();
                qint32 nextHeight = (static_cast<qint32>(textHeight) / dialogHeigth + 1) * dialogHeigth;
                // loop till two lines of text will be shown
                while (m_writePosition < m_Text.size())
                {
                    m_writePosition += 1;
                    m_TextField->setHtmlText(m_Text.mid(0, m_writePosition));
                    textHeight = m_TextField->getTextRect().height();
                    if (textHeight > nextHeight)
                    {
                        m_writePosition -= 1;
                        break;
                    }
                }
                updateShownText();
                if (m_writePosition < m_Text.size())
                {
                    m_paused = true;
                }

            }
        }
    }
}

void GameAnimationDialog::startFinishTimer()
{
    if (!m_stopped && m_started)
    {
        m_finishTimer.setSingleShot(true);
        m_finishTimer.start(m_autoFinishMs);
    }
}

void GameAnimationDialog::update(const oxygine::UpdateState& us)
{
    if (m_started)
    {
        if (m_textTimer.elapsed() > m_textSpeed && !m_paused && !m_stopped)
        {
            m_writePosition += 1;
            // check for auto pause
            float textHeight = m_TextField->getTextRect().height();
            qint32 nextHeight = (static_cast<qint32>(textHeight) / dialogHeigth) * dialogHeigth;
            if (static_cast<qint32>(textHeight) % dialogHeigth != 0)
            {
                nextHeight += dialogHeigth;
            }
            m_TextField->setHtmlText(m_Text.mid(0, m_writePosition));
            textHeight = m_TextField->getTextRect().height();
            if (textHeight > nextHeight && m_autoFinishMs < 0)
            {
                m_writePosition -= 1;
                updateShownText();
                m_paused = true;
            }
            else
            {
                if (m_writePosition > m_Text.size())
                {
                    m_writePosition = m_Text.size();
                    if (m_autoFinishMs >= 0 && !m_finishTimer.isActive())
                    {
                        emit sigStartFinishTimer();
                    }
                }
                else
                {
                    Mainapp* pApp = Mainapp::getInstance();
                    pApp->getAudioManager()->playSound("speaking.wav");
                }
                updateShownText();
            }
            m_textTimer.start();
        }
    }
    GameAnimation::update(us);
}

void GameAnimationDialog::updateShownText()
{
    m_TextField->setHtmlText(m_Text.mid(0, m_writePosition));
    float textHeight = m_TextField->getTextRect().height();
    m_TextField->setHeight(textHeight);
    if (textHeight > dialogHeigth)
    {
        m_TextField->setY((-textHeight + dialogHeigth) - 6);
    }
}

bool GameAnimationDialog::onFinished(bool skipping)
{
    if (m_writePosition >= m_Text.size())
    {
        return GameAnimation::onFinished(skipping);
    }
    else
    {
        m_writePosition = m_Text.size();
    }
    return false;
}

void GameAnimationDialog::setPositionTop(bool value)
{
    if (value)
    {
        setY(0);
        m_BackgroundSprite->setY(0);
    }
    else
    {
        setY(oxygine::Stage::getStage()->getHeight() - m_TextBackground->getScaledHeight());
        m_BackgroundSprite->setY(-getY());
    }
}

void GameAnimationDialog::setColor(QColor color)
{
    m_TextMask->setColor(color);
}

void GameAnimationDialog::setDialog(const QString text)
{
    m_Text = text;
}

void GameAnimationDialog::setCO(const QString coid, GameEnums::COMood mood)
{
    COSpriteManager* pCOSpriteManager = COSpriteManager::getInstance();
    QString resAnim = coid + "+face";
    oxygine::ResAnim* pAnim = pCOSpriteManager->getResAnim(resAnim.toLower());
    if (pAnim != nullptr)
    {
        if (pAnim->getColumns() > 0)
        {
            oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnimColumn(pAnim, static_cast<qint32>(mood)), oxygine::timeMS(static_cast<qint64>(pAnim->getColumns() * GameMap::frameTime)), -1);
            m_COSprite->addTween(tween);
        }
        else
        {
            m_COSprite->setResAnim(pAnim, static_cast<qint32>(mood));
        }
    }
}

void GameAnimationDialog::setPlayerCO(qint32 player, quint8 co, GameEnums::COMood mood)
{
    
    if (m_pMap)
    {
        if (player >= 0 && player < m_pMap->getPlayerCount())
        {
            CO* pCo = m_pMap->getPlayer(player)->getCO(co);
            if (pCo != nullptr)
            {
                QString resAnim = pCo->getCoID() + "+face";
                oxygine::ResAnim* pAnim = pCo->getResAnim(resAnim.toLower());
                if (pAnim != nullptr)
                {
                    m_COSprite->setResAnim(pAnim, static_cast<qint32>(mood));
                }
            }
            else
            {
                m_COSprite->setResAnim(nullptr);
            }
        }
    }
}

void GameAnimationDialog::setFinishDelay(qint32 valueMs)
{
    m_autoFinishMs = valueMs;
}

void GameAnimationDialog::setTextSpeed(qint32 speed)
{
    m_textSpeed = speed / Settings::getInstance()->getAnimationSpeed();
}

void GameAnimationDialog::stop()
{
    GameAnimation::stop();
}

void GameAnimationDialog::restart()
{
    if (m_pMap != nullptr)
    {
        auto* pMenu = m_pMap->getMenu();
        m_writePosition = 0;
        m_stopped = false;
        GameAnimation::restart();
        if (pMenu != nullptr)
        {
            pMenu->addChild(getSharedPtr<oxygine::Actor>());
        }
    }
}

void GameAnimationDialog::start()
{
    if (!m_started)
    {
        GameAnimation::start();
        m_textTimer.start();
    }
}

void GameAnimationDialog::loadBackground(const QString file)
{
    if (!file.isEmpty())
    {
        QImage img;
        QString imgPath = Filesupport::locateResource(file);
        if (QFile::exists(imgPath))
        {
            img = QImage(imgPath);
        }
        oxygine::spSingleResAnim pAnim = MemoryManagement::create<oxygine::SingleResAnim>();
        Mainapp::getInstance()->loadResAnim(pAnim, img, 1, 1, 1);
        m_BackgroundAnim = pAnim;
        m_BackgroundSprite->setResAnim(m_BackgroundAnim.get());
        if (pAnim.get() != nullptr)
        {
            m_BackgroundSprite->setScaleX(static_cast<float>(oxygine::Stage::getStage()->getWidth()) / static_cast<float>(pAnim->getWidth()));
            m_BackgroundSprite->setScaleY(static_cast<float>(oxygine::Stage::getStage()->getHeight()) / static_cast<float>(pAnim->getHeight()));
        }
    }
    else
    {
        CONSOLE_PRINT("Ignoring loading of empty image. GameAnimationDialog::loadBackground", GameConsole::eDEBUG);
    }
}

void GameAnimationDialog::loadCoSprite(const QString coid, float offsetX, float offsetY, bool flippedX, float scale)
{
    if (!coid.isEmpty())
    {
        oxygine::ResAnim* pAnim = COSpriteManager::getInstance()->getResAnim(coid + "+nrm", oxygine::error_policy::ep_ignore_error);
        if (pAnim != nullptr)
        {
            oxygine::spSprite pSprite = MemoryManagement::create<oxygine::Sprite>();
            pSprite->setSize(pAnim->getSize());
            pSprite->setFlippedX(flippedX);
            pSprite->setScale(scale);
            pSprite->setResAnim(pAnim);
            pSprite->setPosition(offsetX, offsetY);
            addChild(pSprite);
        }
    }
}
