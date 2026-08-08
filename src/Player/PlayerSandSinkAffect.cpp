#include "Player/PlayerSandSinkAffect.h"

#include "Library/Area/AreaObjUtil.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"

#include "Player/PlayerConst.h"
#include "Player/PlayerEffect.h"
#include "Util/ObjUtil.h"

PlayerSandSinkAffect::PlayerSandSinkAffect(const al::LiveActor* player, const PlayerConst* pConst,
                                             const PlayerInput* input,
                                             IUsePlayerCollision* collider, PlayerEffect* effect)
    : mPlayer(player), mConst(pConst), mInput(input), mCollider(collider), mEffect(effect) {}

void PlayerSandSinkAffect::clear() {
    if (mIsSafe || mSinkVelocity > 0.0f)
        mEffect->tryDeleteSandSinkEffect();
    mIsSafe = mSinkVelocity > 0.0f;
    mSinkVelocity = 0.0f;
}

bool PlayerSandSinkAffect::isSink() const {
    return mSinkVelocity > 0.0f;
}

bool PlayerSandSinkAffect::isSinkDeathHeight() const {
    return mSinkVelocity + mSinkAmount >= mConst->getSandSinkHeight();
}

void PlayerSandSinkAffect::reduceVelocity(sead::Vector3f* velocity) {
    f32 rawRateH = mSinkVelocity / static_cast<f32>(mConst->getSandSinkBrakeHeightH());
    f32 rateV = 0.0f;
    f32 rateH = 0.0f;
    if (rawRateH < 0.0f) {
    } else {
        rateH = rawRateH;
        if (rawRateH > 1.0f)
            rateH = 1.0f;
    }

    f32 rawRateV = mSinkVelocity / static_cast<f32>(mConst->getSandSinkBrakeHeightV());
    if (rawRateV < 0.0f) {
    } else {
        rateV = rawRateV;
        if (rawRateV > 1.0f)
            rateV = 1.0f;
    }

    f32 brakeHRate = 1.0f - al::easeOut(rateH);
    f32 brakeH = mConst->getSandSinkBrakeMaxH();
    f32 brakeMinH = mConst->getSandSinkBrakeMinH();
    if (brakeHRate < brakeH) {
    } else {
        brakeH = brakeHRate;
        if (brakeHRate > brakeMinH)
            brakeH = brakeMinH;
    }

    f32 brakeVRate = 1.0f - al::easeIn(rateV);
    f32 brakeV = mConst->getSandSinkBrakeMaxV();
    f32 brakeMinV = mConst->getSandSinkBrakeMinV();
    if (brakeVRate < brakeV) {
    } else {
        brakeV = brakeVRate;
        if (brakeVRate > brakeMinV)
            brakeV = brakeMinV;
    }

    sead::Vector3f groundDir;
    groundDir.set(0.0f, 0.0f, 0.0f);
    rs::calcGroundNormalOrGravityDir(&groundDir, mPlayer, mCollider);
    if (groundDir.dot(*velocity) < 0.0f)
        brakeV = 1.0f;

    al::limitVectorParallelVertical(velocity, groundDir,
                                    mConst->getJumpPowerMax() * brakeV,
                                    mConst->getNormalMaxSpeed() * brakeH);
}

bool PlayerSandSinkAffect::isEnableCapThrow() const {
    if (!(mSinkVelocity > 0.0f))
        return true;
    return mSinkVelocity < mConst->getSandSinkCapThrow();
}

f32 PlayerSandSinkAffect::calcSandSinkDeathRate() const {
    return al::calcRate01(mSinkVelocity + mSinkAmount, 0.0f, mConst->getSandSinkHeight());
}

bool isInMePlayArea(const al::LiveActor* player) {
    const al::IUseAreaObj* areaUser = player;
    return al::tryFindAreaObj(areaUser, "PlayerMePlayArea", al::getTrans(player)) != nullptr;
}
