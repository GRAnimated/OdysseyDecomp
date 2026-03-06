#include "Npc/TalkNpcStateEvent.h"

#include "Library/Event/EventFlowExecutor.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/ActorStateReactionBase.h"
#include "Npc/BirdMtxGlideCtrl.h"
#include "Npc/RandomActionUpdater.h"
#include "Npc/TalkNpcActionAnimInfo.h"
#include "Npc/TalkNpcParam.h"
#include "Npc/TalkNpcPartialAnimCtrl.h"
#include "Util/NpcEventFlowUtil.h"

namespace {
NERVE_IMPL(TalkNpcStateEvent, Reaction);
NERVE_END_IMPL(TalkNpcStateEvent, Active);
NERVE_IMPL(TalkNpcStateEvent, StopByTraffic);
NERVE_IMPL(TalkNpcStateEvent, Demo);
NERVE_IMPL(TalkNpcStateEvent, ScareStart);
NERVE_IMPL(TalkNpcStateEvent, Scare);
NERVE_IMPL(TalkNpcStateEvent, ScareEnd);
NERVE_IMPL(TalkNpcStateEvent, BalloonAction);

NERVES_MAKE_NOSTRUCT(TalkNpcStateEvent, Reaction);
NERVES_MAKE_STRUCT(TalkNpcStateEvent, Active, StopByTraffic, Demo, ScareStart, Scare, ScareEnd,
                   BalloonAction);
}  // namespace

TalkNpcStateEvent::TalkNpcStateEvent(const char* name, al::LiveActor* actor,
                                     const TalkNpcParam* param, al::EventFlowExecutor* executor,
                                     s32 a6)
    : al::ActorStateBase(name, actor), mTalkNpcParam(param), mEventFlowExecutor(executor),
      _58(a6) {
    initNerve(&NrvTalkNpcStateEvent.Active, 1);
}

void TalkNpcStateEvent::initStateReaction(ActorStateReactionBase* reaction) {
    mStateReaction = reaction;
    al::initNerveState(this, reaction, &Reaction, u8"リアクション");
}

void TalkNpcStateEvent::initAfterPlacement() {
    if (al::isActive(mEventFlowExecutor))
        return;
    if (!al::isExistEventEntry(mEventFlowExecutor, "InitAfterPlacement"))
        return;
    rs::startEventFlow(mEventFlowExecutor, "InitAfterPlacement");
    rs::updateEventFlow(mEventFlowExecutor);
}

void TalkNpcStateEvent::appear() {
    mIsDead = false;
    if (!al::isActive(mEventFlowExecutor))
        rs::startEventFlow(mEventFlowExecutor, "Init");
    if (mAnimInfo) {
        if (mIsHackWait)
            mAnimInfo->onHackWaitActionName(mTalkNpcParam);
        else
            mAnimInfo->offHackWaitActionName(mTalkNpcParam);
    }
    rs::restartEventFlow(mEventFlowExecutor);
    al::setNerve(this, &NrvTalkNpcStateEvent.Active);
}

void TalkNpcStateEvent::kill() {
    mIsDead = true;
    rs::stopEventFlow(mEventFlowExecutor);
}

bool TalkNpcStateEvent::update() {
    if (_5d) {
        bool isPermitted = rs::tryPermitEnterTrafficNpcAndSyncDrawClipping(mActor);
        bool isStopByTraffic = al::isNerve(this, &NrvTalkNpcStateEvent.StopByTraffic);
        if (isPermitted) {
            if (isStopByTraffic) {
                rs::restartEventFlow(mEventFlowExecutor);
                rs::restartTrafficRailByTraffic(mActor);
                if (!tryStartScare(this, mEventFlowExecutor, mPartialAnimCtrl)) {
                    if (rs::isActiveEventDemo(mActor) && rs::isEqualEventDemoStartActor(mActor))
                        al::setNerve(this, &NrvTalkNpcStateEvent.Demo);
                    else
                        al::setNerve(this, &NrvTalkNpcStateEvent.Active);
                }
            }
        } else if (!isStopByTraffic) {
            rs::stopEventFlow(mEventFlowExecutor);
            rs::stopTrafficRailByTraffic(mActor);
            al::setNerve(this, &NrvTalkNpcStateEvent.StopByTraffic);
        }
    }
    return al::NerveStateBase::update();
}

bool TalkNpcStateEvent::tryStartScare(TalkNpcStateEvent* state,
                                      al::EventFlowExecutor* executor,
                                      TalkNpcPartialAnimCtrl* partialAnimCtrl) {
    if (!al::isScare(executor))
        return false;
    rs::stopEventFlow(executor);
    if (partialAnimCtrl)
        partialAnimCtrl->forceEndAndInvalidateByeBye();
    if (al::isExistEventAction(state->mActor, executor, "ScaredStart") &&
        al::isExistEventAction(state->mActor, executor, "ScaredEnd"))
        al::setNerve(state, &NrvTalkNpcStateEvent.ScareStart);
    else
        al::setNerve(state, &NrvTalkNpcStateEvent.Scare);
    return true;
}

bool TalkNpcStateEvent::trySwitchEventEntry(const char* entryName) {
    if (al::isActive(mEventFlowExecutor) && al::isCurrentEventEntry(mEventFlowExecutor, entryName))
        return false;
    if (!al::isExistEventEntry(mEventFlowExecutor, entryName))
        return false;
    rs::startEventFlow(mEventFlowExecutor, entryName);
    return true;
}

bool TalkNpcStateEvent::trySwitchEventEntryAndUpdate(const char* entryName) {
    if (al::isActive(mEventFlowExecutor) && al::isCurrentEventEntry(mEventFlowExecutor, entryName))
        return false;
    if (!al::isExistEventEntry(mEventFlowExecutor, entryName))
        return false;
    rs::startEventFlow(mEventFlowExecutor, entryName);
    rs::updateEventFlow(mEventFlowExecutor);
    return true;
}

void TalkNpcStateEvent::switchEventEntry(const char* entryName) {
    if (al::isActive(mEventFlowExecutor) && al::isCurrentEventEntry(mEventFlowExecutor, entryName))
        return;
    if (al::isExistEventEntry(mEventFlowExecutor, entryName))
        rs::startEventFlow(mEventFlowExecutor, entryName);
}

bool TalkNpcStateEvent::tryStartReaction(const al::SensorMsg* msg, al::HitSensor* other,
                                         al::HitSensor* self) {
    if (al::isNerve(this, &Reaction) && al::isNewNerve(this))
        return false;
    if (!mStateReaction->receiveMsg(msg, other, self))
        return false;
    rs::stopEventFlow(mEventFlowExecutor);
    if (mGlideCtrl)
        mGlideCtrl->invalidateGlide();
    al::setNerve(this, &Reaction);
    return true;
}

bool TalkNpcStateEvent::tryStartReactionWithoutTrample(const al::SensorMsg* msg,
                                                       al::HitSensor* other,
                                                       al::HitSensor* self) {
    if (al::isNerve(this, &Reaction) && al::isNewNerve(this))
        return false;
    if (!mStateReaction->receiveMsgWithoutTrample(msg, other, self))
        return false;
    rs::stopEventFlow(mEventFlowExecutor);
    if (mGlideCtrl)
        mGlideCtrl->invalidateGlide();
    al::setNerve(this, &Reaction);
    return true;
}

bool TalkNpcStateEvent::isCapReaction() const {
    return mStateReaction->isCapReaction();
}

bool TalkNpcStateEvent::isScare() const {
    return al::isNerve(this, &NrvTalkNpcStateEvent.ScareStart) ||
           al::isNerve(this, &NrvTalkNpcStateEvent.Scare) ||
           al::isNerve(this, &NrvTalkNpcStateEvent.ScareEnd);
}

void TalkNpcStateEvent::exeActive() {
    if (al::isFirstStep(this) && mPartialAnimCtrl) {
        mPartialAnimCtrl->_20 = !al::isWaitAtPointMovement(mEventFlowExecutor);
        mPartialAnimCtrl->_21 = false;
    }
    if (rs::isActiveEventDemo(mActor) && rs::isEqualEventDemoStartActor(mActor)) {
        al::setNerve(this, &NrvTalkNpcStateEvent.Demo);
        return;
    }
    if (tryStartScare(this, mEventFlowExecutor, mPartialAnimCtrl))
        return;
    if (rs::updateEventFlow(mEventFlowExecutor) && _5c) {
        kill();
        return;
    }
    RandomActionUpdater* randomAction = mRandomActionUpdater;
    if (randomAction) {
        randomAction->update();
        if (randomAction->isPlayingBalloonAction()) {
            rs::stopEventFlow(mEventFlowExecutor);
            al::setNerve(this, &NrvTalkNpcStateEvent.BalloonAction);
            return;
        }
    }
    if (al::isFirstStep(this) && mGlideCtrl)
        mGlideCtrl->validateGlide();
}

void TalkNpcStateEvent::endActive() {
    if (mPartialAnimCtrl)
        mPartialAnimCtrl->forceEndAndInvalidateReaction();
}

// NON_MATCHING: extra mov x1, x19 before isEnableReactionRestartEvent call (dead regalloc)
void TalkNpcStateEvent::exeReaction() {
    if (al::isFirstStep(this)) {
        if (mPartialAnimCtrl)
            mPartialAnimCtrl->forceEndAndInvalidateByeBye();
        if (mRandomActionUpdater)
            mRandomActionUpdater->forceEnd();
    }
    if (rs::isActiveEventDemo(mActor) && rs::isEqualEventDemoStartActor(mActor)) {
        al::setNerve(this, &NrvTalkNpcStateEvent.Demo);
        return;
    }
    if (mTalkNpcParam->isEnableReactionRestartEvent(this)) {
        if (tryStartScare(this, mEventFlowExecutor, mPartialAnimCtrl))
            return;
        if (rs::updateEventFlow(mEventFlowExecutor) && _5c) {
            kill();
            return;
        }
        if (rs::isActiveEventDemo(mActor) && rs::isEqualEventDemoStartActor(mActor)) {
            al::setNerve(this, &NrvTalkNpcStateEvent.Demo);
            return;
        }
    }
    if (al::updateNerveState(this)) {
        if (tryStartScare(this, mEventFlowExecutor, mPartialAnimCtrl))
            return;
        al::EventFlowExecutor* executor = mEventFlowExecutor;
        if (mRandomActionUpdater)
            mRandomActionUpdater->restart();
        rs::restartEventFlow(executor);
        al::setNerve(this, &NrvTalkNpcStateEvent.Active);
    }
}

void TalkNpcStateEvent::exeScareStart() {
    if (al::isFirstStep(this)) {
        if (mGlideCtrl)
            mGlideCtrl->invalidateGlide();
        if (mRandomActionUpdater)
            mRandomActionUpdater->forceEnd();
        al::startEventAction(mActor, mEventFlowExecutor, "ScaredStart");
    }
    if (rs::updateEventFlow(mEventFlowExecutor) && _5c) {
        kill();
        return;
    }
    if ((!al::isPlayingEventAction(mActor, mEventFlowExecutor, "ScaredStart") ||
         al::isActionEnd(mActor))) {
        if (al::isScare(mEventFlowExecutor))
            al::setNerve(this, &NrvTalkNpcStateEvent.Scare);
        else if (rs::isActiveEventDemo(mActor) && rs::isEqualEventDemoStartActor(mActor))
            al::setNerve(this, &NrvTalkNpcStateEvent.Demo);
        else
            al::setNerve(this, &NrvTalkNpcStateEvent.ScareEnd);
    }
}

void TalkNpcStateEvent::exeScare() {
    if (al::isFirstStep(this)) {
        if (mGlideCtrl)
            mGlideCtrl->invalidateGlide();
        if (mRandomActionUpdater)
            mRandomActionUpdater->forceEnd();
        al::startEventAction(mActor, mEventFlowExecutor, "Scared");
    }
    if (rs::updateEventFlow(mEventFlowExecutor) && _5c) {
        kill();
        return;
    }
    if (al::isScare(mEventFlowExecutor))
        return;
    if (rs::isActiveEventDemo(mActor) && rs::isEqualEventDemoStartActor(mActor)) {
        al::setNerve(this, &NrvTalkNpcStateEvent.Demo);
    } else if (al::isExistEventAction(mActor, mEventFlowExecutor, "ScaredEnd")) {
        al::setNerve(this, &NrvTalkNpcStateEvent.ScareEnd);
    } else {
        al::EventFlowExecutor* executor = mEventFlowExecutor;
        if (mRandomActionUpdater)
            mRandomActionUpdater->restart();
        rs::restartEventFlow(executor);
        al::setNerve(this, &NrvTalkNpcStateEvent.Active);
    }
}

void TalkNpcStateEvent::exeScareEnd() {
    if (al::isFirstStep(this))
        al::startEventAction(mActor, mEventFlowExecutor, "ScaredEnd");
    if (rs::updateEventFlow(mEventFlowExecutor) && _5c) {
        kill();
        return;
    }
    if (tryStartScare(this, mEventFlowExecutor, mPartialAnimCtrl))
        return;
    if ((!al::isPlayingEventAction(mActor, mEventFlowExecutor, "ScaredEnd") ||
         al::isActionEnd(mActor))) {
        if (rs::isActiveEventDemo(mActor) && rs::isEqualEventDemoStartActor(mActor)) {
            al::setNerve(this, &NrvTalkNpcStateEvent.Demo);
        } else {
            al::EventFlowExecutor* executor = mEventFlowExecutor;
            if (mRandomActionUpdater)
                mRandomActionUpdater->restart();
            rs::restartEventFlow(executor);
            al::setNerve(this, &NrvTalkNpcStateEvent.Active);
        }
    }
}

void TalkNpcStateEvent::exeDemo() {
    if (al::isFirstStep(this)) {
        if (mPartialAnimCtrl)
            mPartialAnimCtrl->forceEndAndInvalidateByeBye();
        if (mRandomActionUpdater)
            mRandomActionUpdater->forceEnd();
    }
    if (rs::updateEventFlow(mEventFlowExecutor) && _5c) {
        kill();
        return;
    }
    if (rs::isActiveEventDemo(mActor) && rs::isEqualEventDemoStartActor(mActor))
        return;
    if (tryStartScare(this, mEventFlowExecutor, mPartialAnimCtrl))
        return;
    rs::restartEventFlow(mEventFlowExecutor);
    al::setNerve(this, &NrvTalkNpcStateEvent.Active);
}

void TalkNpcStateEvent::exeStopByTraffic() {
    if (al::isFirstStep(this)) {
        al::tryStartEventActionIfNotPlaying(mActor, mEventFlowExecutor, "Wait");
        if (mRandomActionUpdater)
            mRandomActionUpdater->restart();
    }
    if (rs::updateEventFlow(mEventFlowExecutor) && _5c)
        kill();
}

void TalkNpcStateEvent::exeBalloonAction() {
    if (rs::isActiveEventDemo(mActor) && rs::isEqualEventDemoStartActor(mActor)) {
        al::setNerve(this, &NrvTalkNpcStateEvent.Demo);
        return;
    }
    if (tryStartScare(this, mEventFlowExecutor, mPartialAnimCtrl))
        return;
    if (rs::updateEventFlow(mEventFlowExecutor) && _5c) {
        kill();
        return;
    }
    mRandomActionUpdater->update();
    if (!mRandomActionUpdater->isPlayingBalloonAction()) {
        al::EventFlowExecutor* executor = mEventFlowExecutor;
        if (mRandomActionUpdater)
            mRandomActionUpdater->restart();
        rs::restartEventFlow(executor);
        al::setNerve(this, &NrvTalkNpcStateEvent.Active);
    }
}
