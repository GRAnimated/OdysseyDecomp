#include "Npc/ShellActorStateReaction.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/ActorStateReactionUtil.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(ShellActorStateReaction, ReactionStart);
NERVE_IMPL(ShellActorStateReaction, ReactionWait);
NERVE_IMPL(ShellActorStateReaction, ReactionShell);
NERVE_IMPL(ShellActorStateReaction, ReactionCapShell);
NERVE_IMPL(ShellActorStateReaction, ReactionCap);
NERVE_IMPL(ShellActorStateReaction, ReactionEnd);
NERVES_MAKE_STRUCT(ShellActorStateReaction, ReactionStart, ReactionWait, ReactionShell,
                   ReactionCapShell, ReactionCap, ReactionEnd);
}  // namespace

ShellActorStateReaction::ShellActorStateReaction(al::LiveActor* actor)
    : ActorStateReactionBase("殻篭りリアクション", actor) {
    initNerve(&NrvShellActorStateReaction.ReactionStart, 0);
}

// NON_MATCHING: branch direction inversion on mIsDead check (cbz vs cbnz) causes layout diff
bool ShellActorStateReaction::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                                         al::HitSensor* self) {
    if (!rs::checkMsgNpcTrampleReactionAll(msg, other, self, false))
        return receiveMsgWithoutTrample(msg, other, self);

    rs::requestHitReactionToAttacker(msg, self, other);

    if (mIsDead) {
        al::setVelocityZero(mActor);
        al::setNerve(this, &NrvShellActorStateReaction.ReactionStart);
    } else if (al::isNerve(this, &NrvShellActorStateReaction.ReactionWait) ||
               al::isNerve(this, &NrvShellActorStateReaction.ReactionShell) ||
               al::isNerve(this, &NrvShellActorStateReaction.ReactionCapShell)) {
        al::setNerve(this, &NrvShellActorStateReaction.ReactionShell);
    } else if (al::isNerve(this, &NrvShellActorStateReaction.ReactionCap) ||
               al::isNerve(this, &NrvShellActorStateReaction.ReactionEnd)) {
        al::setVelocityZero(mActor);
        al::setNerve(this, &NrvShellActorStateReaction.ReactionStart);
    }
    return true;
}

// NON_MATCHING: branch direction inversion on mIsDead check (cbz vs cbnz) causes layout diff
bool ShellActorStateReaction::receiveMsgWithoutTrample(const al::SensorMsg* msg,
                                                       al::HitSensor* other,
                                                       al::HitSensor* self) {
    if (!rs::isMsgNpcCapReactionAll(msg) && !rs::isMsgKillerAttackNoExplode(msg) &&
        !rs::isMsgSphinxRideAttackReflect(msg) && !rs::isMsgTankBulletNoReaction(msg))
        return false;

    rs::requestHitReactionToAttacker(msg, self, other);

    if (rs::isMsgKillerAttackNoExplode(msg) && !mIsDead) {
        if (al::isNerve(this, &NrvShellActorStateReaction.ReactionCap) ||
            al::isNerve(this, &NrvShellActorStateReaction.ReactionWait) ||
            al::isNerve(this, &NrvShellActorStateReaction.ReactionShell) ||
            al::isNerve(this, &NrvShellActorStateReaction.ReactionCapShell)) {
            al::setNerve(this, &NrvShellActorStateReaction.ReactionCapShell);
            return false;
        }
    }

    if (mIsDead) {
        al::setVelocityZero(mActor);
        al::setNerve(this, &NrvShellActorStateReaction.ReactionCap);
        return true;
    }

    if (al::isNerve(this, &NrvShellActorStateReaction.ReactionStart) ||
        al::isNerve(this, &NrvShellActorStateReaction.ReactionWait) ||
        al::isNerve(this, &NrvShellActorStateReaction.ReactionShell)) {
        al::setNerve(this, &NrvShellActorStateReaction.ReactionCapShell);
        return true;
    }

    if (ActorStateReactionUtil::isInvalidRestartCapReaction(msg, this))
        return true;

    if (al::isNerve(this, &NrvShellActorStateReaction.ReactionCapShell)) {
        al::setNerve(this, &NrvShellActorStateReaction.ReactionCapShell);
        return true;
    }

    al::setVelocityZero(mActor);
    al::setNerve(this, &NrvShellActorStateReaction.ReactionCap);
    return true;
}

bool ShellActorStateReaction::receiveMsgNoReaction(const al::SensorMsg* msg, al::HitSensor* other,
                                                   al::HitSensor* self) {
    return rs::isMsgSphinxRideAttack(msg);
}

bool ShellActorStateReaction::isCapReaction() const {
    if (al::isNerve(this, &NrvShellActorStateReaction.ReactionCap))
        return true;
    return al::isNerve(this, &NrvShellActorStateReaction.ReactionCapShell);
}

void ShellActorStateReaction::exeReactionStart() {
    if (al::isFirstStep(this))
        al::startAction(mActor, "ReactionStart");
    if (al::isActionEnd(mActor))
        al::setNerve(this, &NrvShellActorStateReaction.ReactionWait);
}

void ShellActorStateReaction::exeReactionWait() {
    if (al::isFirstStep(this))
        al::startAction(mActor, "ReactionWait");
    f32 dist = al::calcDistanceH(mActor, rs::getPlayerPos(mActor));
    if (dist > 1000.0f)
        al::setNerve(this, &NrvShellActorStateReaction.ReactionEnd);
    else
        al::setNerveAtGreaterEqualStep(this, &NrvShellActorStateReaction.ReactionEnd, 240);
}

void ShellActorStateReaction::exeReactionShell() {
    if (al::isFirstStep(this))
        al::startAction(mActor, "ReactionShell");
    if (al::isActionEnd(mActor))
        al::setNerve(this, &NrvShellActorStateReaction.ReactionWait);
}

void ShellActorStateReaction::exeReactionCap() {
    if (al::isFirstStep(this))
        al::startAction(mActor, "ReactionCap");
    if (al::isActionEnd(mActor))
        kill();
}

void ShellActorStateReaction::exeReactionCapShell() {
    if (al::isFirstStep(this))
        al::startAction(mActor, "ReactionCapShell");
    if (al::isActionEnd(mActor))
        al::setNerve(this, &NrvShellActorStateReaction.ReactionWait);
}

void ShellActorStateReaction::exeReactionEnd() {
    if (al::isFirstStep(this))
        al::startAction(mActor, "ReactionEnd");
    if (al::isActionEnd(mActor))
        kill();
}
