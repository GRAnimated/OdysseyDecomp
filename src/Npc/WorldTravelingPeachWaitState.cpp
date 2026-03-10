#include "Npc/WorldTravelingPeachWaitState.h"

#include <math/seadVector.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/NpcStateTurnSeparate.h"
#include "Npc/TalkNpcActionAnimInfo.h"
#include "Npc/Tiara.h"
#include "Util/FukankunZoomTargetFunction.h"
#include "Util/NpcAnimUtil.h"
#include "Util/PlayerUtil.h"

namespace {
NERVE_IMPL(WorldTravelingPeachWaitState, Wait);
NERVE_IMPL(WorldTravelingPeachWaitState, TurnToTiara);
NERVE_IMPL(WorldTravelingPeachWaitState, TurnToBack);
NERVE_IMPL(WorldTravelingPeachWaitState, FindTurn);
NERVE_IMPL(WorldTravelingPeachWaitState, TurnToPlayer);
NERVE_IMPL(WorldTravelingPeachWaitState, WaitInitPeachWorldHomeCastleCapDemo);
NERVE_IMPL(WorldTravelingPeachWaitState, AdlibTalk);
NERVE_IMPL(WorldTravelingPeachWaitState, ShakeHand);
NERVE_IMPL(WorldTravelingPeachWaitState, ShakeHandPeachWorldHomeCastleCapDemo);
NERVE_IMPL(WorldTravelingPeachWaitState, WaitAfterPeachWorldHomeCastleCapDemo);

NERVES_MAKE_NOSTRUCT(WorldTravelingPeachWaitState, Wait, TurnToTiara, TurnToBack, FindTurn,
                     TurnToPlayer, WaitInitPeachWorldHomeCastleCapDemo, AdlibTalk, ShakeHand,
                     ShakeHandPeachWorldHomeCastleCapDemo, WaitAfterPeachWorldHomeCastleCapDemo);
}  // namespace

WorldTravelingPeachWaitState::WorldTravelingPeachWaitState(al::LiveActor* actor, Tiara* tiara,
                                                           const TalkNpcParam* param,
                                                           const TalkNpcActionAnimInfo* animInfo)
    : al::EventFlowMovement(u8"世界旅行ピーチの待機挙動", actor), mTiara(tiara),
      mTalkNpcParam(param), mAnimInfo(animInfo), mFrontDir(0.0f, 0.0f, 0.0f), _5c(false) {
    al::calcFrontDir(&mFrontDir, mActor);
    initNerve(&Wait, 4);

    mTurnSeparate = new NpcStateTurnSeparate(u8"ターン", actor);
    mTurnSeparate->initEventUserForAction(this);

    al::initNerveState(this, mTurnSeparate, &TurnToTiara, u8"ティアラにターン");
    al::initNerveState(this, mTurnSeparate, &TurnToBack, u8"前の方向にターン");
    al::initNerveState(this, mTurnSeparate, &FindTurn, u8"発見ターン");
    al::initNerveState(this, mTurnSeparate, &TurnToPlayer, u8"プレイヤーにターン");
}

void WorldTravelingPeachWaitState::appear() {
    if (!al::isNerve(this, &WaitInitPeachWorldHomeCastleCapDemo))
        al::setNerve(this, &Wait);
}

void WorldTravelingPeachWaitState::kill() {
    mTiara->startWait();
    rs::resetNpcEyeLineAnim(mActor);
}

void WorldTravelingPeachWaitState::startPeachWorldHomeCastleCapDemo() {
    al::faceToDirection(mActor, mFrontDir);
    al::faceToDirection(mTiara, mFrontDir);
    mTiara->startPeachWorldHomeCastleCapDemo();
    al::setNerve(this, &WaitInitPeachWorldHomeCastleCapDemo);
}

void WorldTravelingPeachWaitState::endPeachWorldHomeCastleCapDemo() {
    mTiara->endPeachWorldHomeCastleCapDemo();
    al::setNerve(this, &Wait);
}

void WorldTravelingPeachWaitState::exeWait() {
    if (al::isFirstStep(this))
        rs::startNpcAction(mActor, mAnimInfo->getWaitActionName());

    al::LiveActor* actor = mActor;
    const TalkNpcParam* talkParam = mTalkNpcParam;
    const sead::Vector3f& playerPos = rs::getPlayerPos(actor);
    rs::tryUpdateNpcEyeLineAnimToTarget(actor, talkParam, playerPos, true);

    if (!_5c) {
        NpcStateTurnSeparate* turnSep = mTurnSeparate;
        if (rs::isNearPlayerH(mActor, 2000.0f) ||
            FukankunZoomTargetFunction::getWatchCount(mActor) >= 91) {
            turnSep->startTurnToTarget(rs::getPlayerPos(mActor));
            al::setNerve(this, &FindTurn);
            return;
        }
    }

    if (_5c) {
        NpcStateTurnSeparate* turnSep = mTurnSeparate;
        if (turnSep->tryStartTurnToTarget(rs::getPlayerPos(mActor), 75.0f)) {
            al::setNerve(this, &TurnToPlayer);
            return;
        }
    }

    s32 frameMax = (s32)al::getActionFrameMax(mActor, al::getActionName(mActor));
    if (al::isGreaterEqualStep(this, 3 * frameMax)) {
        f32 modiResult = (f32)al::modi(frameMax + (s32)al::getActionFrame(mActor) + 1, frameMax);
        if (modiResult < al::getActionFrame(mActor)) {
            if (al::getRandom() < 0.4f) {
                mTurnSeparate->startTurnToTarget(al::getTrans(mTiara));
                al::setNerve(this, &TurnToTiara);
            }
        }
    }
}

void WorldTravelingPeachWaitState::exeTurnToTiara() {
    rs::resetNpcEyeLineAnim(mActor);
    al::updateNerveStateAndNextNerve(this, &AdlibTalk);

    if (al::isFirstStep(this)) {
        Tiara* tiara = mTiara;
        f32 frameMax = al::getActionFrameMax(mActor, al::getActionName(mActor));
        tiara->startTurnToHostAndStop((s32)frameMax);
    }
}

void WorldTravelingPeachWaitState::exeAdlibTalk() {
    if (al::isFirstStep(this)) {
        rs::startNpcAction(mActor, "AdlibTalk");
        mTiara->startStopAction("AdlibTalk");
    }

    rs::resetNpcEyeLineAnim(mActor);

    if (!_5c) {
        NpcStateTurnSeparate* turnSep = mTurnSeparate;
        if (rs::isNearPlayerH(mActor, 2000.0f) ||
            FukankunZoomTargetFunction::getWatchCount(mActor) >= 91) {
            turnSep->startTurnToTarget(rs::getPlayerPos(mActor));
            al::setNerve(this, &FindTurn);
            return;
        }
    }

    if (!al::isActionPlaying(mTiara, "AdlibTalk") || al::isActionEnd(mActor)) {
        mTurnSeparate->startTurnToDir(mFrontDir);
        al::setNerve(this, &TurnToBack);
    }
}

void WorldTravelingPeachWaitState::exeTurnToBack() {
    if (al::isFirstStep(this))
        mTiara->startWait();

    rs::resetNpcEyeLineAnim(mActor);

    if (_5c) {
        al::updateNerveStateAndNextNerve(this, &Wait);
        return;
    }

    NpcStateTurnSeparate* turnSep = mTurnSeparate;
    if (rs::isNearPlayerH(mActor, 2000.0f) ||
        FukankunZoomTargetFunction::getWatchCount(mActor) >= 91) {
        turnSep->startTurnToTarget(rs::getPlayerPos(mActor));
        al::setNerve(this, &FindTurn);
        return;
    }

    al::updateNerveStateAndNextNerve(this, &Wait);
}

void WorldTravelingPeachWaitState::exeFindTurn() {
    if (al::isFirstStep(this)) {
        mTiara->startFindTurn();
        _5c = true;
    }

    rs::resetNpcEyeLineAnim(mActor);
    al::updateNerveStateAndNextNerve(this, &ShakeHand);
}

void WorldTravelingPeachWaitState::exeShakeHand() {
    if (al::isFirstStep(this))
        rs::startNpcAction(mActor, "ShakeHand");

    rs::tryUpdateNpcEyeLineAnim(mActor, mTalkNpcParam);

    if (al::isActionEnd(mActor)) {
        mTurnSeparate->startTurnToDir(mFrontDir);
        al::setNerve(this, &TurnToBack);
    }
}

void WorldTravelingPeachWaitState::exeTurnToPlayer() {
    rs::tryUpdateNpcEyeLineAnim(mActor, mTalkNpcParam);
    al::updateNerveStateAndNextNerve(this, &Wait);
}

void WorldTravelingPeachWaitState::exeWaitInitPeachWorldHomeCastleCapDemo() {
    if (al::isFirstStep(this))
        rs::startNpcAction(mActor, "Wait");

    al::setNerveAtGreaterEqualStep(this, &ShakeHandPeachWorldHomeCastleCapDemo, 90);
}

void WorldTravelingPeachWaitState::exeShakeHandPeachWorldHomeCastleCapDemo() {
    if (al::isFirstStep(this)) {
        rs::startNpcAction(mActor, "ShakeHand");
        mTiara->startShakeHandPeachWorldHomeCastleCapDemo();
    }

    if (al::isActionEnd(mActor))
        al::setNerve(this, &WaitAfterPeachWorldHomeCastleCapDemo);
}

void WorldTravelingPeachWaitState::exeWaitAfterPeachWorldHomeCastleCapDemo() {
    if (al::isFirstStep(this))
        rs::startNpcAction(mActor, "Wait");
}
