#include "Player/YoshiStateHackDown.h"

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionAirMoveControl.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
class PlayerActionAirMoveControlOverlay {
public:
    u8 _0[0x20];
    IUsePlayerHack** mPlayerHack;
    u8 _28[0x18];
    bool _40;
};

NERVE_IMPL(YoshiStateHackDown, Down)
NERVE_IMPL(YoshiStateHackDown, Land)
NERVES_MAKE_NOSTRUCT(YoshiStateHackDown, Down, Land)
}  // namespace

YoshiStateHackDown::YoshiStateHackDown(al::LiveActor* actor, IUsePlayerHack** playerHack,
                                       const PlayerConst* playerConst,
                                       const IUsePlayerCollision* collision,
                                       PlayerAnimator* animator)
    : HackerStateBase("ダウン", actor, playerHack), mPlayerConst(playerConst),
      mCollision(collision), mAnimator(animator), mAirMoveControl(nullptr),
      mGroundNormal(0.0f, 0.0f, 0.0f) {
    mAirMoveControl = new PlayerActionAirMoveControl(actor, playerConst, nullptr, collision, false);
    auto* airMove = reinterpret_cast<PlayerActionAirMoveControlOverlay*>(mAirMoveControl);
    airMove->_40 = true;
    airMove->mPlayerHack = playerHack;
    initNerve(&Down, 0);
}

void YoshiStateHackDown::appear() {
    al::LiveActor* actor = mActor;
    HackerStateBase::appear();
    rs::calcGroundNormalOrGravityDir(&mGroundNormal, actor, mCollision);

    sead::Vector3f front(0.0f, 0.0f, 0.0f);
    al::calcFrontDir(&front, actor);
    al::setVelocity(actor, -mPlayerConst->getPushPowerDamage() * front);

    mAirMoveControl->setup(mPlayerConst->getJumpMoveSpeedMax(),
                           mPlayerConst->getPushPowerDamage(), 0,
                           mPlayerConst->getHopPowerDamage(), mPlayerConst->getGravityDamage(), 9999,
                           1.0f);
    al::setNerve(this, &Down);
}

bool YoshiStateHackDown::isLand() const {
    return al::isNerve(this, &Land);
}

bool YoshiStateHackDown::isEnableCancel() const {
    if (!al::isNerve(this, &Land))
        return false;
    return al::isGreaterEqualStep(this, mPlayerConst->getDamageCancelFrame());
}

void YoshiStateHackDown::exeDown() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this))
        mAnimator->startAnim("NoDamageDown");

    mAirMoveControl->update();
    if (rs::isOnGroundLessAngle(actor, mCollision, mPlayerConst->getStandAngleMin())) {
        rs::startHitReactionLandIfLanding(actor, mCollision, false);
        rs::brakeLandVelocityGroundNormal(actor, &mGroundNormal, mCollision, -al::getGravity(actor),
                                          0.0f, mPlayerConst->getGravity());
        al::setNerve(this, &Land);
    } else if (al::isGreaterEqualStep(this, 120)) {
        kill();
    }
}

void YoshiStateHackDown::exeLand() {
    if (al::isFirstStep(this))
        mAnimator->startAnim("DamageLand");

    al::LiveActor* actor = mActor;
    if (rs::isOnGroundLessAngle(actor, mCollision, mPlayerConst->getStandAngleMin())) {
        rs::brakeLandVelocityGroundNormal(actor, &mGroundNormal, mCollision, mGroundNormal, 0.0f,
                                          mPlayerConst->getGravity());
    } else {
        al::tryAddVelocityLimit(actor, al::getGravity(actor) * mPlayerConst->getGravityAir(),
                                mPlayerConst->getFallSpeedMax());
    }

    if (mAnimator->isAnimEnd())
        kill();
}

YoshiStateHackDown::~YoshiStateHackDown() = default;
