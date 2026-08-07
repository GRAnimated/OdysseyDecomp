#include "Player/YoshiStateEgg.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAreaFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseKeeper.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerConst.h"
#include "Player/PlayerModelChangerYoshi.h"
#include "Player/YoshiEgg.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerHackFunction.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
void updateEggMovement(al::LiveActor* actor, const IUsePlayerCollision* collision,
                       const PlayerConst* playerConst);

NERVE_IMPL(YoshiStateEgg, Wait);
NERVE_IMPL(YoshiStateEgg, Appear);
NERVES_MAKE_NOSTRUCT(YoshiStateEgg, Wait, Appear);
}  // namespace

YoshiStateEgg::YoshiStateEgg(const al::ActorInitInfo& info, al::LiveActor* actor,
                             const IUsePlayerCollision* collision,
                             const PlayerConst* playerConst,
                             const al::WaterSurfaceFinder* waterSurfaceFinder,
                             PlayerModelChangerYoshi* modelChanger)
    : al::ActorStateBase("", actor), mCollision(collision), mPlayerConst(playerConst),
      mWaterSurfaceFinder(waterSurfaceFinder), mModelChanger(modelChanger) {
    mEgg = new YoshiEgg(actor, collision);
    mEgg->init(info);
    initNerve(&Wait, 0);
}

void YoshiStateEgg::appear() {
    al::ActorStateBase::appear();
    mEgg->kill();
    al::invalidateHitSensors(mActor);
    if (mIsFirstAppear) {
        mIsFirstAppear = false;
        mEgg->initPlacementEgg();
        al::setNerve(this, &Wait);
    } else {
        al::setNerve(this, &Appear);
    }
}

void YoshiStateEgg::kill() {
    al::ActorStateBase::kill();
    al::validateHitSensors(mActor);
}

void YoshiStateEgg::exeAppear() {
    if (al::isFirstStep(this))
        mEgg->appearEgg();
    updateEggMovement(mActor, mCollision, mPlayerConst);
    if (mEgg->isEndAppear())
        al::setNerve(this, &Wait);
}

namespace {
void updateEggMovement(al::LiveActor* actor, const IUsePlayerCollision* collision,
                       const PlayerConst* playerConst) {
    if (rs::isCollidedGround(collision)) {
        rs::waitGround(actor, collision, playerConst->getGravity(), playerConst->getFallSpeedMax(),
                       playerConst->getSlerpQuatGrav(), 0.0f);
    } else {
        sead::Vector3f gravity = al::getGravity(actor) * playerConst->getGravity();
        al::tryAddVelocityLimit(actor, gravity, playerConst->getFallSpeedMax());
    }
}
}  // namespace

void YoshiStateEgg::exeWait() {
    if (al::isFirstStep(this))
        al::validateClipping(mActor);
    updateEggMovement(mActor, mCollision, mPlayerConst);
    if (mEgg->isBreak()) {
        mModelChanger->appearModel();
        kill();
    }
}

bool YoshiStateEgg::reactionCollidedCollisionCode() {
    if (isDead())
        return false;

    const al::LiveActor* actor = mActor;
    if (!al::isInDeathArea(actor) &&
        (!al::isNerve(this, &Wait) ||
         (!rs::isTouchHackCancelCollisionCode(actor, mCollision) && !al::isInWater(actor))))
        return false;

    al::startHitReaction(actor, "[ヨッシー]死亡");
    return true;
}
