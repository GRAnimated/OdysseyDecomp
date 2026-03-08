#include "Npc/NpcStateReactionParam.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/ActorStateReactionUtil.h"
#include "Npc/NpcStateReaction.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(NpcStateReaction, Reaction);
NERVE_IMPL(NpcStateReaction, CapReaction);
NERVES_MAKE_NOSTRUCT(NpcStateReaction, Reaction, CapReaction);
}  // namespace

static NpcStateReactionParam sDefaultParam;

NpcStateReaction* NpcStateReaction::create(al::LiveActor* actor,
                                           const NpcStateReactionParam* param) {
    auto* state = new NpcStateReaction(actor, false);
    if (param)
        state->mParam = param;
    return state;
}

NpcStateReaction* NpcStateReaction::createForHuman(al::LiveActor* actor,
                                                   const NpcStateReactionParam* param) {
    auto* state = new NpcStateReaction(actor, false);
    state->_29 = true;
    if (param)
        state->mParam = param;
    return state;
}

NpcStateReaction* NpcStateReaction::createForHackActor(al::LiveActor* actor,
                                                       const NpcStateReactionParam* param) {
    auto* state = new NpcStateReaction(actor, true);
    if (param)
        state->mParam = param;
    return state;
}

NpcStateReaction::NpcStateReaction(al::LiveActor* actor, bool isCapReaction)
    : ActorStateReactionBase("Npc", actor) {
    mParam = &sDefaultParam;
    mIsCapReaction = isCapReaction;
    _29 = false;
    _2a = false;
    mWasClippingInvalid = false;
    initNerve(&Reaction, 0);
}

void NpcStateReaction::appear() {
    mIsDead = false;
    if (al::isInvalidClipping(mActor))
        mWasClippingInvalid = true;
    else {
        al::invalidateClipping(mActor);
        mWasClippingInvalid = false;
    }
}

void NpcStateReaction::kill() {
    mIsDead = true;
    if (!mWasClippingInvalid)
        al::validateClipping(mActor);
    mWasClippingInvalid = false;
}

bool NpcStateReaction::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                                  al::HitSensor* self) {
    if (!rs::checkMsgNpcTrampleReactionAll(msg, other, self, _29))
        return receiveMsgWithoutTrample(msg, other, self);

    if (rs::isPlayerBinding(mActor))
        return false;

    if (!mIsDead && al::isNerve(this, &Reaction) && al::isLessStep(this, 2))
        return false;

    if (mIsDead || !al::isNerve(this, &Reaction) || al::isGreaterEqualStep(this, 8)) {
        if (!_29)
            rs::requestHitReactionToAttacker(msg, self, other);
    }

    al::setNerve(this, &Reaction);
    return true;
}

bool NpcStateReaction::receiveMsgWithoutTrample(const al::SensorMsg* msg, al::HitSensor* other,
                                                al::HitSensor* self) {
    if ((_2a && rs::isMsgTankBulletNoReaction(msg)) || rs::isMsgNpcCapReactionAll(msg) ||
        rs::isMsgKillerAttackNoExplode(msg) || rs::isMsgSphinxRideAttackReflect(msg)) {
        if (mIsCapReaction && rs::isMsgCapReflect(msg))
            return false;

        if (rs::isMsgKillerAttackNoExplode(msg) && !mIsDead && al::isNerve(this, &CapReaction))
            return false;

        if (!mIsDead && al::isNerve(this, &CapReaction)) {
            if (rs::isMsgPlayerObjectWallHit(msg))
                return false;
            if (ActorStateReactionUtil::isInvalidRestartCapReaction(msg, this))
                return true;
        }

        if (!_29)
            rs::requestHitReactionToAttacker(msg, self, other);

        al::setNerve(this, &CapReaction);
        return true;
    }

    rs::isMsgMotorcycleDashAttack(msg);
    return false;
}

bool NpcStateReaction::receiveMsgNoReaction(const al::SensorMsg* msg, al::HitSensor* other,
                                            al::HitSensor* self) {
    return rs::isMsgSphinxRideAttack(msg);
}

bool NpcStateReaction::isCapReaction() const {
    return al::isNerve(this, &CapReaction);
}

void NpcStateReaction::exeReaction() {
    if (al::isFirstStep(this)) {
        al::startAction(mActor, mParam->mReactionAnim.cstr());
        al::setVelocityZero(mActor);
    }
    if (al::isActionEnd(mActor))
        kill();
}

void NpcStateReaction::exeCapReaction() {
    if (al::isFirstStep(this)) {
        al::startAction(mActor, mParam->mReactionEndAnim.cstr());
        al::setVelocityZero(mActor);
    }
    if (al::isActionEnd(mActor))
        kill();
}

NpcStateReactionParam::NpcStateReactionParam()
    : mReactionAnim("Reaction"), mReactionEndAnim("ReactionCap") {}

NpcStateReactionParam::NpcStateReactionParam(const char* reactionAnim,
                                             const char* reactionEndAnim)
    : mReactionAnim(reactionAnim), mReactionEndAnim(reactionEndAnim) {}
