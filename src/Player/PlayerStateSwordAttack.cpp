#include "Player/PlayerStateSwordAttack.h"

#include "Library/Base/StringUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
NERVE_IMPL(PlayerStateSwordAttack, Attack)
NERVES_MAKE_STRUCT(PlayerStateSwordAttack, Attack)
}  // namespace

PlayerStateSwordAttack::PlayerStateSwordAttack(al::LiveActor* player, al::LiveActor* sword)
    : al::ActorStateBase("", player), mSword(sword), mIsPowerGrove(false) {
    if (sword)
        mIsPowerGrove = al::isEqualString(al::getModelName(sword), "PowerGrove");
    initNerve(&NrvPlayerStateSwordAttack.Attack, 0);
}

PlayerStateSwordAttack::~PlayerStateSwordAttack() = default;

void PlayerStateSwordAttack::appear() {
    al::NerveStateBase::appear();
    al::setNerve(this, &NrvPlayerStateSwordAttack.Attack);
}

void PlayerStateSwordAttack::kill() {
    al::NerveStateBase::kill();
    if (mSword)
        al::invalidateHitSensors(mSword);
}

// NON_MATCHING: exact size matches but target keeps the ground flag live across both branches;
// move the shared action-end test after the movement branch to mirror the corpus CFG.
void PlayerStateSwordAttack::exeAttack() {
    if (al::isFirstStep(this)) {
        const char* actionName;
        if (mIsPowerGrove)
            actionName = "Punch";
        else
            actionName = "Fire";
        al::startAction(mActor, actionName);
    }

    if (al::isStep(this, 0) && mSword)
        al::validateHitSensors(mSword);

    u32 isOnGround = al::isOnGround(mActor, 2);
    if (isOnGround) {
        const sead::Vector3f* groundNormal = &al::getCollidedGroundNormal(mActor);
        al::addVelocityToDirection(mActor, -*groundNormal, 1.0f);
        al::scaleVelocityExceptDirection(mActor, -*groundNormal, 0.9f);
        al::reboundVelocityFromCollision(mActor, 0.0f, 0.0f, 1.0f);
    } else {
        al::addVelocityToGravity(mActor, 1.0f);
        al::LiveActor* gravityActor = mActor;
        al::scaleVelocityDirection(gravityActor, al::getGravity(gravityActor), 0.99f);
    }

    bool isActionEnd = al::isActionEnd(mActor);
    if ((isOnGround & isActionEnd) != 0)
        kill();
}
