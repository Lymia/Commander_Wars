#include "3rd_party/oxygine-framework/oxygine/actor/Stage.h"
#include "3rd_party/oxygine-framework/oxygine/actor/Box9Sprite.h"
#include "3rd_party/oxygine-framework/oxygine/tween/tweenwait.h"
#include "3rd_party/oxygine-framework/oxygine/tween/tweenscreenshake.h"

#include "game/gameanimation/gameanimation.h"
#include "game/gameanimation/gameanimationfactory.h"

#include "resource_management/gameanimationmanager.h"
#include "resource_management/fontmanager.h"

#include "coreengine/filesupport.h"
#include "coreengine/gameconsole.h"
#include "coreengine/interpreter.h"
#include "coreengine/settings.h"
#include "coreengine/audiomanager.h"

GameAnimation::GameAnimation(quint32 frameTime, GameMap* pMap)
    : m_frameTime(frameTime / Settings::getInstance()->getAnimationSpeed()),
      m_pMap{pMap}
{
#ifdef GRAPHICSUPPORT
    setObjectName("GameAnimation");
#endif
    Q_ASSERT(Mainapp::getInstance()->renderingPaused());
    Interpreter::setCppOwnerShip(this);
    m_actionData.setVersion(QDataStream::Version::Qt_6_5);
    if (m_frameTime <= 0)
    {
        m_frameTime = 1;
    }
    connect(this, &GameAnimation::sigFinished, this, &GameAnimation::onFinished, Qt::QueuedConnection);
    connect(this, &GameAnimation::sigStart, this, &GameAnimation::start, Qt::QueuedConnection);
    m_buffer.open(QIODevice::ReadWrite);
    oxygine::Sprite::setVisible(false);
}

void GameAnimation::restart()
{
    Q_ASSERT(Mainapp::getInstance()->renderingPaused());
    if (m_pMap != nullptr)
    {
        Mainapp::getInstance()->pauseRendering();
        m_stopped = false;
        m_previousAnimation.reset();
        for (auto & tween : m_stageTweens)
        {
            oxygine::Stage::getStage()->addTween(tween);
        }
        if (m_global)
        {
            oxygine::Stage::getStage()->addChild(getSharedPtr<Actor>());
        }
        else
        {
            m_pMap->addChild(getSharedPtr<Actor>());
        }
        start();
        Mainapp::getInstance()->continueRendering();
    }
}

void GameAnimation::start()
{
    if (!m_started)
    {
        Mainapp::getInstance()->pauseRendering();
        m_started = true;
        setVisible(true);
        m_previousAnimation.reset();
        doPreAnimationCall();
        AudioManager* pAudioThread = Mainapp::getInstance()->getAudioManager();
        for (auto & data : m_SoundData)
        {
            pAudioThread->playSound(data.soundFile, data.loops, static_cast<float>(data.delayMs) / Settings::getInstance()->getAnimationSpeed(), data.volume, data.stopOldestSound);
        }
        Mainapp::getInstance()->continueRendering();
    }
}

void GameAnimation::doPreAnimationCall()
{
    if ((!m_jsPreActionObject.isEmpty()) && (!m_jsPreActionObject.isEmpty()))
    {
        CONSOLE_PRINT("Calling post Animation function " + m_jsPreActionObject + "." + m_jsPreActionFunction, GameConsole::eDEBUG);
        Interpreter* pInterpreter = Interpreter::getInstance();
        QJSValueList args({pInterpreter->newQObject(this),
                           GameMap::getMapJsThis(m_pMap)});
        pInterpreter->doFunction(m_jsPreActionObject, m_jsPreActionFunction, args);
    }
}

bool GameAnimation::getGlobal() const
{
    return m_global;
}

void GameAnimation::setGlobal(bool newGlobal)
{
    m_global = newGlobal;
}

GameAnimation* GameAnimation::getPreviousAnimation() const
{
    return m_previousAnimation.get();
}

bool GameAnimation::getStarted() const
{
    return m_started;
}

GameMap *GameAnimation::getMap() const
{
    return m_pMap;
}

void GameAnimation::stop()
{
    Mainapp::getInstance()->pauseRendering();
    Q_ASSERT(!m_started);
    m_stopped = true;
    setVisible(false);
    Mainapp::getInstance()->continueRendering();
}

qint32 GameAnimation::getFontWidth(const QString & font, const QString & text) const
{
    oxygine::TextStyle headline = oxygine::TextStyle(FontManager::getInstance()->getFont(font));
    headline.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    headline.multiline = false;
    oxygine::TextField testField;
    testField.setStyle(headline);
    testField.setHtmlText(text);
    return testField.getTextRect().width();
}

void GameAnimation::setRotation(float angle)
{
    setRotationDegrees(angle);
}

void GameAnimation::queueAnimation(GameAnimation* pGameAnimation)
{
    Q_ASSERT(Mainapp::getInstance()->renderingPaused());
    if (pGameAnimation != nullptr)
    {
        pGameAnimation->setPreviousAnimation(getSharedPtr<GameAnimation>());
        m_QueuedAnimations.append(pGameAnimation->getSharedPtrFromWeak<GameAnimation>());
        GameAnimationFactory::getInstance()->queueAnimation(pGameAnimation);
    }
}

void GameAnimation::queueAnimationBefore(GameAnimation* pGameAnimation)
{
    if (m_previousAnimation.get() != nullptr)
    {
        // remove ourself from previous animation and add our ancestor
        m_previousAnimation->removeQueuedAnimation(this);
        m_previousAnimation->queueAnimation(pGameAnimation);
    }
    // queue ourself after the given animation
    pGameAnimation->queueAnimation(this);
}

void GameAnimation::removeQueuedAnimation(GameAnimation* pGameAnimation)
{
    qint32 i = 0;
    while (i < m_QueuedAnimations.size())
    {
        if (m_QueuedAnimations[i].get() == pGameAnimation)
        {
            m_QueuedAnimations.removeAt(i);
        }
        else
        {
            i++;
        }
    }
}

void GameAnimation::update(const oxygine::UpdateState& us)
{
    Q_ASSERT(!m_stopped);
    if (m_started)
    {
        oxygine::Sprite::update(us);
    }
    else if (!m_startEmitted)
    {
        m_startEmitted = true;
        emit sigStart();
    }
}

bool GameAnimation::getStopSoundAtAnimationEnd() const
{
    return m_stopSoundAtAnimationEnd;
}

void GameAnimation::setStopSoundAtAnimationEnd(bool stopSoundAtAnimationEnd)
{
    m_stopSoundAtAnimationEnd = stopSoundAtAnimationEnd;
}

bool GameAnimation::getVisible() const
{
    return oxygine::Sprite::getVisible();
}
void GameAnimation::setVisible(bool vis)
{
    oxygine::Sprite::setVisible(vis);
}

void GameAnimation::setPreviousAnimation(spGameAnimation previousAnimation)
{
    m_previousAnimation = previousAnimation;
}

void GameAnimation::addSprite(QString spriteID, float offsetX, float offsetY, qint32 sleepAfterFinish, float scale, qint32 delay, qint32 loops)
{
    addSprite2(spriteID, offsetX, offsetY, sleepAfterFinish, scale, scale, delay, loops);
}

void GameAnimation::addSprite2(QString spriteID, float offsetX, float offsetY, qint32 sleepAfterFinish, float scaleX, float scaleY, qint32 delay, qint32 loops)
{
    addSprite3(spriteID, offsetX, offsetY, QColor(255, 255, 255), sleepAfterFinish, scaleX, scaleY, delay, 0, loops);
}

void GameAnimation::addSpriteAnimTable(QString spriteID, float offsetX, float offsetY, Player* pPlayer, qint32 sleepAfterFinish, float scaleX, float scaleY, qint32 delay, qint32, GameEnums::Recoloring mode)
{
    GameAnimationManager* pGameAnimationManager = GameAnimationManager::getInstance();
    oxygine::ResAnim* pAnim = pGameAnimationManager->getResAnim(spriteID, oxygine::error_policy::ep_ignore_error);
    if (pAnim != nullptr)
    {
        loadSpriteAnimTable(pAnim, offsetX, offsetY, pPlayer, sleepAfterFinish, scaleX, scaleY, delay, mode);
    }
    else
    {
        CONSOLE_PRINT_MODULE("Unable to load animation sprite: " + spriteID, GameConsole::eDEBUG, GameConsole::eResources);
    }
}

void GameAnimation::addSprite3(QString spriteID, float offsetX, float offsetY, QColor color, qint32 sleepAfterFinish, float scaleX, float scaleY, qint32 delay, qint32 frames, qint32 loops, QEasingCurve::Type easeType)
{
    GameAnimationManager* pGameAnimationManager = GameAnimationManager::getInstance();
    oxygine::ResAnim* pAnim = pGameAnimationManager->getResAnim(spriteID, oxygine::error_policy::ep_ignore_error);
    if (pAnim != nullptr)
    {
        loadSpriteAnim(pAnim, offsetX, offsetY, color, sleepAfterFinish, scaleX, scaleY, delay, loops);
    }
    else
    {
        QImage img;
        QString imgPath = Filesupport::locateResource(spriteID);
        if (QFile::exists(imgPath))
        {
            img = QImage(imgPath);
        }
        else
        {
            CONSOLE_PRINT_MODULE("Unable to load animation sprite: " + spriteID, GameConsole::eDEBUG, GameConsole::eResources);
            return;
        }
        oxygine::spSingleResAnim pAnim = MemoryManagement::create<oxygine::SingleResAnim>();
        Mainapp::getInstance()->loadResAnim(pAnim, img, 1, frames, 1);
        m_resAnims.append(pAnim);
        loadSpriteAnim(pAnim.get(), offsetX, offsetY, color, sleepAfterFinish, scaleX, scaleY, delay, loops, easeType);
    }
}

void GameAnimation::addBox(QString spriteID, float offsetX, float offsetY, qint32 width, qint32 heigth,  qint32 sleepAfterFinish, QColor color)
{
    GameAnimationManager* pGameAnimationManager = GameAnimationManager::getInstance();
    oxygine::ResAnim* pAnim = pGameAnimationManager->getResAnim(spriteID, oxygine::error_policy::ep_ignore_error);
    if (pAnim != nullptr)
    {
        oxygine::spBox9Sprite pBox = MemoryManagement::create<oxygine::Box9Sprite>();
        m_lastCreatedTweenQueue = MemoryManagement::create<oxygine::TweenQueue>();
        pBox->setResAnim(pAnim);
        pBox->setSize(width, heigth);
        setSize(width, heigth);
        if (sleepAfterFinish > 0)
        {
            oxygine::spTween tween1 = oxygine::createTween(TweenWait(), oxygine::timeMS(static_cast<qint64>(static_cast<float>(sleepAfterFinish) / Settings::getInstance()->getAnimationSpeed())), 1);
            m_lastCreatedTweenQueue->add(tween1);
        }
        pBox->addTween(m_lastCreatedTweenQueue);
        if (color != Qt::white)
        {
            pBox->setColor(color);
        }
        addChild(pBox);
        pBox->setPosition(offsetX, offsetY);
        if (pBox->getScaledWidth() - offsetX > getWidth())
        {
            setWidth(pBox->getScaledWidth() - offsetX);
        }
        if (pBox->getScaledHeight() - offsetY > getHeight())
        {
            setHeight(pBox->getScaledHeight() - offsetY);
        }
        if(!m_finishQueued)
        {
            m_finishQueued = true;
            m_lastCreatedTweenQueue->setDoneCallback([this](oxygine::Event *)->void
            {
                emitFinished();
            });
        }
    }
}

void GameAnimation::loadSpriteAnimTable(oxygine::ResAnim* pAnim, float offsetX, float offsetY, Player* pPlayer, qint32 sleepAfterFinish, float scaleX, float scaleY, qint32 delay, GameEnums::Recoloring mode)
{
    if (pAnim != nullptr)
    {
        oxygine::spSprite pSprite = MemoryManagement::create<oxygine::Sprite>();
        oxygine::spTweenQueue queuedAnim = MemoryManagement::create<oxygine::TweenQueue>();
        oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnim), oxygine::timeMS(pAnim->getTotalFrames() * m_frameTime), 1, false, oxygine::timeMS(static_cast<qint64>(static_cast<float>(delay) / Settings::getInstance()->getAnimationSpeed())));
        queuedAnim->add(tween);
        if (sleepAfterFinish > 0)
        {
            oxygine::spTween tween1 = oxygine::createTween(TweenWait(), oxygine::timeMS(static_cast<qint64>(static_cast<float>(sleepAfterFinish) / Settings::getInstance()->getAnimationSpeed())), 1);
            queuedAnim->add(tween1);
        }
        pSprite->setScaleX(scaleX);
        pSprite->setScaleY(scaleY);
        pSprite->addTween(queuedAnim);
        bool matrixMode = mode == GameEnums::Recoloring_Matrix;
        if (pPlayer != nullptr)
        {
            pSprite->setColorTable(pPlayer->getColorTableAnim(), matrixMode);
        }
        else
        {
            pSprite->setColorTable(Player::getNeutralTableAnim(), matrixMode);
        }
        addChild(pSprite);
        pSprite->setPosition(offsetX, offsetY);
        if(!m_finishQueued)
        {
            m_finishQueued = true;
            queuedAnim->setDoneCallback([this](oxygine::Event *)->void
            {
                emitFinished();
            });
        }
    }
}

void GameAnimation::loadSpriteAnim(oxygine::ResAnim* pAnim, float offsetX, float offsetY, QColor color, qint32 sleepAfterFinish, float scaleX, float scaleY, qint32 delay, qint32 loops, QEasingCurve::Type easeType)
{
    if (pAnim != nullptr)
    {
        oxygine::spSprite pSprite = MemoryManagement::create<oxygine::Sprite>();
        m_lastCreatedTweenQueue = MemoryManagement::create<oxygine::TweenQueue>();
        qint32 totalFrames = 0;
        if (pAnim != nullptr)
        {
            totalFrames = pAnim->getTotalFrames();
        }
        oxygine::spTween tween = oxygine::createTween(oxygine::TweenAnim(pAnim), oxygine::timeMS(totalFrames * m_frameTime), loops, false, oxygine::timeMS(static_cast<qint64>(static_cast<float>(delay) / Settings::getInstance()->getAnimationSpeed())), easeType);
        m_lastCreatedTweenQueue->add(tween);
        if (sleepAfterFinish > 0)
        {
            oxygine::spTween tween1 = oxygine::createTween(TweenWait(), oxygine::timeMS(static_cast<qint64>(static_cast<float>(sleepAfterFinish) / Settings::getInstance()->getAnimationSpeed())), 1);
            m_lastCreatedTweenQueue->add(tween1);
        }
        setSize(pAnim->getSize());
        pSprite->setScaleX(scaleX);
        pSprite->setScaleY(scaleY);
        pSprite->setSize(pAnim->getSize());
        pSprite->addTween(m_lastCreatedTweenQueue);
        if (color != Qt::white)
        {
            pSprite->setColor(color);
        }
        addChild(pSprite);
        pSprite->setPosition(offsetX, offsetY);

        if (pSprite->getScaledWidth() - offsetX > getWidth())
        {
            setWidth(pSprite->getScaledWidth() - offsetX);
        }
        if (pSprite->getScaledHeight() - offsetY > getHeight())
        {
            setHeight(pSprite->getScaledHeight() - offsetY);
        }

        if(!m_finishQueued)
        {
            m_finishQueued = true;
            m_lastCreatedTweenQueue->setDoneCallback([this](oxygine::Event *)->void
            {
                emitFinished();
            });
        }
    }
}

qint32 GameAnimation::addText(QString text, float offsetX, float offsetY, float scale, QColor color, qint32 priority)
{
    oxygine::TextStyle style = oxygine::TextStyle(FontManager::getFont("animationFont24"));
    style.color = color;
    style.hAlign = oxygine::TextStyle::HALIGN_LEFT;
    style.multiline = false;
    oxygine::spTextField pTextfield = MemoryManagement::create<oxygine::TextField>();
    pTextfield->setStyle(style);
    pTextfield->setHtmlText(text);
    pTextfield->setPosition(offsetX, offsetY);
    pTextfield->setPriority(priority);
    pTextfield->setWidth(pTextfield->getTextRect().width());
    pTextfield->setHeight(40);
    pTextfield->setScale(scale);
    addChild(pTextfield);
    return pTextfield->getTextRect().width();
}

bool GameAnimation::onFinished(bool skipping)
{
    Q_ASSERT(m_started);
    if (!m_finished)
    {
        m_finished = true;
        Mainapp::getInstance()->pauseRendering();
        m_skipping |= skipping;
        GameAnimationFactory::removeAnimationFromQueue(getSharedPtr<GameAnimation>());
        if (!m_started)
        {
            doPreAnimationCall();
        }
        for (auto & data : m_SoundData)
        {
            if (m_stopSoundAtAnimationEnd || skipping || data.loops < 0)
            {
                Mainapp::getInstance()->getAudioManager()->stopSound(data.soundFile);
            }
        }
        GameAnimationFactory* pGameAnimationFactory = GameAnimationFactory::getInstance();
        for (qint32 i = 0; i < m_QueuedAnimations.size(); i++)
        {
            pGameAnimationFactory->startQueuedAnimation(m_QueuedAnimations[i].get());
        }
        m_QueuedAnimations.clear();
        if ((!m_jsPostActionObject.isEmpty()) && (!m_jsPostActionObject.isEmpty()))
        {
            CONSOLE_PRINT("Calling post Animation function " + m_jsPostActionObject + "." + m_jsPostActionFunction, GameConsole::eDEBUG);
            Interpreter* pInterpreter = Interpreter::getInstance();
            QJSValueList args({pInterpreter->newQObject(this),
                               GameMap::getMapJsThis(m_pMap)});
            pInterpreter->doFunction(m_jsPostActionObject, m_jsPostActionFunction, args);
        }
        m_stageTweens.clear();
        GameAnimationFactory::removeAnimation(getSharedPtr<GameAnimation>(), m_skipping, false);
        if (!skipping && GameAnimationFactory::getAnimationCount() > 0)
        {
            GameAnimationFactory::skipAllAnimations();
            if (GameAnimationFactory::getAnimationCount() == 0)
            {
                emit GameAnimationFactory::getInstance()->animationsFinished();
            }
        }
        Mainapp::getInstance()->continueRendering();
    }
    return true;
}

void GameAnimation::setSound(QString soundFile, qint32 loops, qint32 delayMs, float volume, bool stopOldestSound)
{
    if (m_SoundData.size() == 0)
    {
        addSound(soundFile, loops, delayMs, volume);
    }
    else
    {
        SoundData data;
        data.soundFile = soundFile;
        data.loops = loops;
        data.volume = volume;
        data.delayMs = delayMs;
        data.stopOldestSound = stopOldestSound;
        m_SoundData[0] = data;
    }
}

void GameAnimation::addSound(QString soundFile, qint32 loops, qint32 delayMs, float volume, bool stopOldestSound)
{
    SoundData data;
    data.soundFile = soundFile;
    data.loops = loops;
    data.volume = volume;
    data.delayMs = delayMs;
    data.stopOldestSound = stopOldestSound;
    m_SoundData.append(data);
}

void GameAnimation::addTweenScale(float endScale, qint32 duration, QEasingCurve::Type easeType)
{
    oxygine::spTween tween1 = oxygine::createTween(oxygine::Actor::TweenScale(endScale), oxygine::timeMS(static_cast<qint64>(static_cast<float>(duration) / Settings::getInstance()->getAnimationSpeed())), 1, false, oxygine::timeMS(0), easeType);
    addTween(tween1);
}

void GameAnimation::addTweenPosition(QPoint point, qint32 duration, QEasingCurve::Type easeType)
{
    oxygine::spTween tween1 = oxygine::createTween(oxygine::Actor::TweenPosition(point), oxygine::timeMS(static_cast<qint64>(static_cast<float>(duration) / Settings::getInstance()->getAnimationSpeed())), 1, false, oxygine::timeMS(0), easeType);
    addTween(tween1);
}

void GameAnimation::addTweenColor(qint32 spriteIdx, QColor startColor, QColor endColor, qint32 duration, bool twoSided, qint32 delay, QEasingCurve::Type easeType)
{
    Mainapp::getInstance()->pauseRendering();
    if (spriteIdx < m_children.size())
    {
        oxygine::spActor actor = m_children[spriteIdx];
        oxygine::Sprite* sprite = dynamic_cast<oxygine::Sprite*>(actor.get());
        if (sprite != nullptr)
        {
            sprite->setColor(startColor);
            oxygine::Sprite::TweenColor tweenColor(endColor);
            oxygine::spTween tween = oxygine::createTween(tweenColor, oxygine::timeMS(static_cast<qint64>(static_cast<float>(duration) / Settings::getInstance()->getAnimationSpeed())), 1, twoSided, oxygine::timeMS(static_cast<qint64>(static_cast<float>(delay) / Settings::getInstance()->getAnimationSpeed())), easeType);
            sprite->addTween(tween);
        }
    }
    Mainapp::getInstance()->continueRendering();
}

void GameAnimation::addTweenWait(qint32 duration, QEasingCurve::Type easeType)
{
    oxygine::spTween tween1 = oxygine::createTween(TweenWait(), oxygine::timeMS(static_cast<qint64>(static_cast<float>(duration) / Settings::getInstance()->getAnimationSpeed())), 1, false, oxygine::timeMS(0), easeType);
    addTween(tween1);
    if(!m_finishQueued)
    {
        m_finishQueued = true;
        tween1->setDoneCallback([this](oxygine::Event *)->void
        {
            emitFinished();
        });
    }
}

void GameAnimation::addTweenWaitToLastQueue(qint32 duration, QEasingCurve::Type easeType)
{
    oxygine::spTween tween1 = oxygine::createTween(TweenWait(), oxygine::timeMS(static_cast<qint64>(static_cast<float>(duration) / Settings::getInstance()->getAnimationSpeed())), 1, false, oxygine::timeMS(0), easeType);
    m_lastCreatedTweenQueue->add(tween1);
}

void GameAnimation::addTweenScaleToLastQueue(float endScale, qint32 duration, QEasingCurve::Type easeType)
{
    oxygine::spTween tween1 = oxygine::createTween(oxygine::Actor::TweenScale(endScale), oxygine::timeMS(static_cast<qint64>(static_cast<float>(duration) / Settings::getInstance()->getAnimationSpeed())), 1, false, oxygine::timeMS(0), easeType);
    m_lastCreatedTweenQueue->add(tween1);
}

void GameAnimation::addTweenPositionToLastQueue(QPoint point, qint32 duration, QEasingCurve::Type easeType)
{
    oxygine::spTween tween1 = oxygine::createTween(oxygine::Actor::TweenPosition(point), oxygine::timeMS(static_cast<qint64>(static_cast<float>(duration) / Settings::getInstance()->getAnimationSpeed())), 1, false, oxygine::timeMS(0), easeType);
    m_lastCreatedTweenQueue->add(tween1);
}

void GameAnimation::addTweenColorToLastQueue(QColor endColor, qint32 duration, bool twoSided, qint32 delay, QEasingCurve::Type easeType)
{
    oxygine::Sprite::TweenColor tweenColor(endColor);
    oxygine::spTween tween = oxygine::createTween(tweenColor, oxygine::timeMS(static_cast<qint64>(static_cast<float>(duration) / Settings::getInstance()->getAnimationSpeed())), 1, twoSided, oxygine::timeMS(static_cast<qint64>(static_cast<float>(delay) / Settings::getInstance()->getAnimationSpeed())), easeType);
    m_lastCreatedTweenQueue->add(tween);
}


void GameAnimation::setEndOfAnimationCall(QString postActionObject, QString postActionFunction)
{
    m_jsPostActionObject = postActionObject;
    m_jsPostActionFunction = postActionFunction;
}

void GameAnimation::setStartOfAnimationCall(QString preActionObject, QString preActionFunction)
{
    m_jsPreActionObject = preActionObject;
    m_jsPreActionFunction = preActionFunction;
}

void GameAnimation::emitFinished()
{
    if (!m_finished)
    {
        Q_ASSERT(m_started);
        emit sigFinished(m_skipping);
    }
}

void GameAnimation::addScreenshake(qint32 startIntensity, float decay, qint32 durationMs, qint32 delayMs, qint32 shakePauseMs, QEasingCurve::Type easeType)
{
    oxygine::spTween tween = oxygine::createTween(TweenScreenshake(startIntensity, static_cast<float>(decay) / Settings::getInstance()->getAnimationSpeed(), oxygine::timeMS(shakePauseMs)),
                                                  oxygine::timeMS(static_cast<qint64>(static_cast<float>(durationMs) / Settings::getInstance()->getAnimationSpeed())), 1, false,
                                                  oxygine::timeMS(static_cast<qint64>(static_cast<float>(delayMs) / Settings::getInstance()->getAnimationSpeed())), easeType);
    m_stageTweens.append(tween);
    oxygine::Stage::getStage()->addTween(tween);

}

quint32 GameAnimation::getFrameTime() const
{
    return m_frameTime;
}
