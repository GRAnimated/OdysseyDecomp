#include "Player/PlayerStateRise.h"

#include "Library/Area/AreaObjUtil.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerAnimator.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
NERVE_IMPL(PlayerStateRise, Rise)
NERVES_MAKE_STRUCT(PlayerStateRise, Rise)
}  // namespace

PlayerStateRise::PlayerStateRise(al::LiveActor* player, const PlayerConst* pConst,
                                 const IUsePlayerCollision* collision, const PlayerInput* input,
                                 PlayerAnimator* animator)
    : al::ActorStateBase("浮き上がり", player), mCollision(collision), mAnimator(animator),
      mAirMoveControl(nullptr) {
    mAirMoveControl = new PlayerActionAirMoveControl(player, pConst, input, collision, false);
    initNerve(&NrvPlayerStateRise.Rise, 0);
}

PlayerStateRise::~PlayerStateRise() = default;

void PlayerStateRise::appear() {
    al::NerveStateBase::appear();
    if (rs::isCollidedGround(mCollision)) {
        al::LiveActor* actor = mActor;
        al::AreaObj* area = al::tryFindAreaObj(actor, "RiseArea", al::getTrans(actor));
        sead::Vector3f up = {0.0f, 0.0f, 0.0f};
        al::getAreaObjDirUp(&up, area);
        f32 speed = up.dot(al::getVelocity(actor));
        if (speed < 0.0f)
            al::addVelocityToDirection(actor, up, -speed);
    }
    al::setNerve(this, &NrvPlayerStateRise.Rise);
}

// NON_MATCHING: behavior and surrounding symbols match, but exeRise differs from the target;
// compare clamp/max expression lowering and local vector lifetime against the exact assembly.
void PlayerStateRise::exeRise() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this))
        mAnimator->startAnim("SandWait");

    al::AreaObj* area = al::tryFindAreaObj(actor, "RiseArea", al::getTrans(actor));
    if (area) {
        sead::Vector3f up = {0.0f, 0.0f, 0.0f};
        al::getAreaObjDirUp(&up, area);
        f32 speedV = up.dot(al::getVelocity(actor));
        f32 edgeRate = al::calcNearestAreaObjEdgeRateTopY(area, al::getTrans(actor));
        f32 easedRate = al::easeOut(edgeRate);
        f32 maxPower = speedV < -10.0f ? 10.0f : 2.0f;
        f32 power =
            sead::Mathf::clamp(easedRate * (maxPower - 0.5f) + 0.5f, 0.5f, maxPower);
        al::tryAddVelocityLimit(actor, up * power, 30.0f);

        f32 speedHRaw = al::calcSpeedH(actor);
        f32 speedH = sead::Mathf::clampMin(speedHRaw, 10.0f);
        al::LiveActor* setupActor = mActor;
        PlayerActionAirMoveControl* airMoveControl = mAirMoveControl;
        f32 setupSpeedV = al::calcSpeedV(setupActor);
        airMoveControl->setup(speedH, speedH, 0, setupSpeedV, 0.1f, 0, 0.0f);
    } else {
        al::LiveActor* setupActor = mActor;
        PlayerActionAirMoveControl* airMoveControl = mAirMoveControl;
        f32 setupSpeedV = al::calcSpeedV(setupActor);
        airMoveControl->setup(10.0f, 10.0f, 0, setupSpeedV, 1.0f, 0, 0.0f);
    }
    mAirMoveControl->update();
}
