#include "Boss/BossUtil/BossStateTalkDemo.h"

#include <prim/seadSafeString.h>

#include "Library/Base/StringUtil.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/Event/EventFlowFunction.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorSceneFunction.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Util/DemoUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerDemoUtil.h"

namespace {
NERVE_IMPL(BossStateTalkDemo, Demo);
NERVE_IMPL(BossStateTalkDemo, Skip);
NERVE_IMPL(BossStateTalkDemo, End);
NERVE_IMPL(BossStateTalkDemo, DemoTalkFirst);
NERVE_IMPL(BossStateTalkDemo, DemoTalk);
NERVES_MAKE_NOSTRUCT(BossStateTalkDemo, Demo, Skip, End, DemoTalkFirst, DemoTalk);
}  // namespace

// NON_MATCHING: register allocation (compiler saves x23 as extra callee-saved register)
BossStateTalkDemo* BossStateTalkDemo::createWithEventFlow(al::LiveActor* owner,
                                                          const al::ActorInitInfo& initInfo,
                                                          const char* eventFlowName,
                                                          const char* cameraSuffix) {
    auto* executor = rs::initEventFlow(owner, initInfo, eventFlowName, cameraSuffix);
    auto* state = new BossStateTalkDemo("ボス会話デモ", owner, initInfo, executor);
    return state;
}

// NON_MATCHING: register allocation (compiler saves x23 as extra callee-saved register)
BossStateTalkDemo* BossStateTalkDemo::createWithEventFlow(al::LiveActor* owner,
                                                          const al::ActorInitInfo& initInfo,
                                                          const char* eventFlowName) {
    auto* executor = rs::initEventFlow(owner, initInfo, nullptr, eventFlowName);
    auto* state = new BossStateTalkDemo("ボス会話デモ", owner, initInfo, executor);
    return state;
}

// NON_MATCHING: register allocation (compiler saves x25/x24/x23 as extra callee-saved registers)
//               and bool field initialization uses strh for two fields at once
BossStateTalkDemo::BossStateTalkDemo(const char* name, al::LiveActor* owner,
                                     const al::ActorInitInfo& initInfo,
                                     al::EventFlowExecutor* executor)
    : al::ActorStateBase(name, owner), mEventFlowExecutor(executor) {
    mIsRequestedDemo = true;
    mIsEnableSkipDemo = true;
    mIsFirstDemo = true;
    mIsReplaced = false;
    mReplaceTrans = {0.0f, 0.0f, 0.0f};
    mReplaceQuat = sead::Quatf::unit;

    auto* actionName = new sead::FixedSafeString<64>();
    mDemoActionName = actionName;
    mDemoAnimCamera = al::initDemoAnimCamera(owner, initInfo, "ボス会話デモ");
    initNerve(&Demo, 0);
}

bool BossStateTalkDemo::tryStart(const char* entryName) {
    if (!rs::tryStartEventCutSceneDemo(mActor))
        return false;
    mDemoEntryName = entryName;
    rs::requestValidateDemoSkip(this, mActor);
    al::requestCaptureScreenCover(mActor, 4);
    al::setNerve(this, &Demo);
    return true;
}

void BossStateTalkDemo::startWithoutRequestDemo(const char* entryName) {
    mDemoEntryName = entryName;
    mIsRequestedDemo = false;
    al::setNerve(this, &Demo);
}

void BossStateTalkDemo::setEventReceiver(al::IEventFlowEventReceiver* receiver) {
    al::initEventReceiver(mEventFlowExecutor, receiver);
}

void BossStateTalkDemo::kill() {
    al::NerveStateBase::kill();
    if (al::isHideDemoPlayer(mEventFlowExecutor))
        rs::showDemoPlayer(mActor);
    if (al::isActiveCamera(mDemoAnimCamera))
        al::endCamera(mActor, mDemoAnimCamera, -1, false);
    if (mIsReplaced)
        rs::replaceDemoPlayer(mActor, mReplaceTrans, mReplaceQuat);
    if (mIsRequestedDemo)
        rs::endEventCutSceneDemoOrTryEndEventCutSceneDemoBySkip(mActor);
}

bool BossStateTalkDemo::update() {
    mDemoActionName->clear();
    return al::NerveStateBase::update();
}

bool BossStateTalkDemo::isFirstDemo() const {
    return mIsFirstDemo;
}

bool BossStateTalkDemo::isEnableSkipDemo() const {
    return mIsEnableSkipDemo;
}

void BossStateTalkDemo::skipDemo() {
    if (!mIsEnableSkipDemo)
        return;
    rs::skipEventDemo(mEventFlowExecutor);
    al::setNerve(this, &Skip);
    kill();
}

// NON_MATCHING: register allocation (compiler saves x24/x23 as extra callee-saved registers)
void BossStateTalkDemo::exeDemo() {
    if (al::isFirstStep(this)) {
        if (mDemoEntryName) {
            rs::startEventFlow(mEventFlowExecutor, mDemoEntryName);
            mDemoEntryName = nullptr;
        }
    }
    if (rs::updateEventFlow(mEventFlowExecutor)) {
        al::setNerve(this, &End);
        return;
    }
    if (al::isPlayingEventDemoAction(mEventFlowExecutor)) {
        if (al::isActionEnd(mActor))
            al::endEventDemoAction(mEventFlowExecutor);
    } else if (al::isRequestEventDemoAction(mEventFlowExecutor)) {
        const char* actionName = al::getEventDemoActionName(mEventFlowExecutor);
        al::startAction(mActor, actionName);
        mDemoActionName->format(actionName);
        al::IUseCamera* cameraUser = mActor ? (al::IUseCamera*)mActor : nullptr;
        if (al::isExistAnimCameraData(mDemoAnimCamera, actionName)) {
            if (al::isActiveCamera(mDemoAnimCamera))
                al::startAnimCameraAnim(mDemoAnimCamera, actionName, -1, -1, -1);
            else
                al::startAnimCamera(cameraUser, mDemoAnimCamera, actionName);
        }
        al::startEventDemoAction(mEventFlowExecutor);
    }
    rs::tryHideDemoPlayerIfRequested(mActor, mEventFlowExecutor);
    rs::tryShowDemoPlayerIfRequested(mActor, mEventFlowExecutor);
    rs::tryStartDemoPlayerActionIfRequested(mActor, mEventFlowExecutor);
    if (!rs::isCloseNpcDemoEventTalkMessage(mActor))
        al::setNerve(this, &DemoTalk);
}

// NON_MATCHING: register allocation (compiler saves x23/x22/x21 as extra callee-saved registers)
void BossStateTalkDemo::exeDemoTalkFirst() {
    if (rs::updateEventFlow(mEventFlowExecutor)) {
        al::setNerve(this, &End);
        return;
    }
    const char* talkAction = al::getEventDemoTalkActionName(mEventFlowExecutor);
    if (al::isFirstStep(this)) {
        al::startAction(mActor, talkAction);
        mDemoActionName->format(talkAction);
        al::IUseCamera* cameraUser = mActor ? (al::IUseCamera*)mActor : nullptr;
        if (al::isExistAnimCameraData(mDemoAnimCamera, talkAction)) {
            if (al::isActiveCamera(mDemoAnimCamera))
                al::startAnimCameraAnim(mDemoAnimCamera, talkAction, -1, -1, -1);
            else
                al::startAnimCamera(cameraUser, mDemoAnimCamera, talkAction);
        }
    }
    if (rs::isPlayingTextPaneAnimEventTalkMessage(mActor))
        al::setNerve(this, &DemoTalk);
}

// NON_MATCHING: register allocation (compiler saves x21 as extra callee-saved register)
void BossStateTalkDemo::exeDemoTalk() {
    if (rs::updateEventFlow(mEventFlowExecutor)) {
        al::setNerve(this, &End);
        return;
    }
    if (rs::isCloseNpcDemoEventTalkMessage(mActor)) {
        al::setNerve(this, &Demo);
        return;
    }
    if (!al::isExistEventDemoTalkAction(mEventFlowExecutor))
        return;
    const char* talkAction = al::getEventDemoTalkActionName(mEventFlowExecutor);
    if (rs::isPlayingTextPaneAnimEventTalkMessage(mActor)) {
        if (!al::isActionPlaying(mActor, talkAction)) {
            al::startAction(mActor, talkAction);
            mDemoActionName->format(talkAction);
        }
    } else {
        al::StringTmp<64> waitName("%sWait", talkAction);
        if (!al::isActionPlaying(mActor, waitName.cstr())) {
            al::startAction(mActor, waitName.cstr());
            mDemoActionName->format(waitName.cstr());
        }
    }
}

void BossStateTalkDemo::exeEnd() {
    if (al::isFirstStep(this)) {
        if (al::isActiveCamera(mDemoAnimCamera))
            al::requestCaptureScreenCover(mActor, 4);
        else {
            kill();
            return;
        }
    }
    if (al::isGreaterEqualStep(this, 1))
        kill();
}

void BossStateTalkDemo::exeSkip() {}

bool BossStateTalkDemo::isPrevTalkDemo() const {
    return al::isNerve(this, &Demo);
}

bool BossStateTalkDemo::isStartActionTiming() const {
    return *mDemoActionName->getStringTop() != sead::SafeStringBase<char>::cNullChar;
}

const char* BossStateTalkDemo::getStartActionName() const {
    return mDemoActionName->cstr();
}
