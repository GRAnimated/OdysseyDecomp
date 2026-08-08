#include "Player/YoshiStateHackWallAir.h"

#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/Nerve.h"
#include "Library/Nerve/NerveKeeper.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerAnimator.h"
#include "Player/PlayerWallActionHistory.h"
#include "Player/YoshiStateHackWallCling.h"
#include "Player/YoshiStateHackWallJump.h"
#include "Util/PlayerHackInputFunction.h"

namespace {
class YoshiStateHackWallAirNrvCling : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override {
        keeper->getParent<YoshiStateHackWallAir>()->exeCling();
    }
};

class YoshiStateHackWallAirNrvJump : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override {
        YoshiStateHackWallAir* state = keeper->getParent<YoshiStateHackWallAir>();
        if (al::updateNerveState(state))
            state->kill();
    }
};

NERVES_MAKE_STRUCT(YoshiStateHackWallAir, Cling, Jump)
}  // namespace

YoshiStateHackWallAir::YoshiStateHackWallAir(
    al::LiveActor* actor, IUsePlayerHack** playerHack, const PlayerConst* playerConst,
    const IUsePlayerCollision* collision, const PlayerTrigger* trigger, const YoshiTongue* tongue,
    PlayerWallActionHistory* wallActionHistory, PlayerAnimator* animator)
    : HackerStateBase("空中壁", actor, playerHack), mCollision(collision),
      mWallActionHistory(wallActionHistory), mAnimator(animator), mStateCling(nullptr),
      mStateJump(nullptr) {
    initNerve(&NrvYoshiStateHackWallAir.Cling, 2);
    mStateCling = new YoshiStateHackWallCling(actor, playerHack, playerConst, collision, animator);
    mStateJump =
        new YoshiStateHackWallJump(actor, playerHack, playerConst, collision, trigger, tongue, animator);
    al::initNerveState(this, mStateCling, &NrvYoshiStateHackWallAir.Cling, "壁接着");
    al::initNerveState(this, mStateJump, &NrvYoshiStateHackWallAir.Jump, "壁ジャンプ");
}

void YoshiStateHackWallAir::appear() {
    HackerStateBase::appear();
    mWallActionHistory->reset();
    al::setNerve(this, &NrvYoshiStateHackWallAir.Cling);
}

bool YoshiStateHackWallAir::isCling() const {
    return al::isNerve(this, &NrvYoshiStateHackWallAir.Cling);
}

bool YoshiStateHackWallAir::isAir() const {
    if (!al::isNerve(this, &NrvYoshiStateHackWallAir.Jump))
        return false;
    return al::isGreaterStep(this, 0);
}

void YoshiStateHackWallAir::setupCling(const al::CollisionParts* collisionParts,
                                       const sead::Vector3f& position,
                                       const sead::Vector3f& normal) {
    mStateCling->setup(collisionParts, position, normal);
}

void YoshiStateHackWallAir::startShrink() {
    mWallActionHistory->recordWallJump(mCollision, al::getTrans(mActor));

    if (al::isNerve(this, &NrvYoshiStateHackWallAir.Cling)) {
        al::LiveActor* actor = mActor;
        sead::Vector3f front(0.0f, 0.0f, 0.0f);
        al::calcFrontDir(&front, actor);
        sead::Vector3f up(0.0f, 0.0f, 0.0f);
        al::calcUpDir(&up, actor);
        sead::Quatf quat = sead::Quatf::unit;
        al::makeQuatFrontUp(&quat, -front, up);
        al::updatePoseQuat(actor, quat);
        mAnimator->startAnim("Fall");
    }
}

void YoshiStateHackWallAir::exeCling() {
    if (al::updateNerveState(this)) {
        sead::Vector3f wallNormal(0.0f, 0.0f, 0.0f);
        al::calcFrontDir(&wallNormal, mActor);
        wallNormal.negate();
        mWallActionHistory->recordWallJump(al::getTrans(mActor), wallNormal);
        kill();
        return;
    }

    if (rs::isTriggerHackJump(*mPlayerHack)) {
        mWallActionHistory->recordWallJump(mCollision, al::getTrans(mActor));
        al::setNerve(this, &NrvYoshiStateHackWallAir.Jump);
    }
}

void YoshiStateHackWallAir::exeJump() {
    if (al::updateNerveState(this))
        kill();
}
