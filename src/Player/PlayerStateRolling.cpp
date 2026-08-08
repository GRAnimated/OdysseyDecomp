#include "Player/PlayerStateRolling.h"

#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionSlopeSlideControl.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerEffect.h"
#include "Util/JudgeUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
NERVE_IMPL(PlayerStateRolling, Start);
NERVE_IMPL(PlayerStateRolling, Rolling);
NERVE_IMPL(PlayerStateRolling, BoostStart);
NERVE_IMPL(PlayerStateRolling, Jump);
NERVE_IMPL(PlayerStateRolling, StandUp);

class PlayerStateRollingNrvEndSquat : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override {
        PlayerStateRolling* state = keeper->getParent<PlayerStateRolling>();
        if (al::isFirstStep(state))
            state->getEffect()->tryDeleteRollingEffect();
    }
};

NERVE_IMPL(PlayerStateRolling, UnRoll);
NERVE_IMPL(PlayerStateRolling, Land);
NERVE_IMPL(PlayerStateRolling, Brake);

NERVES_MAKE_STRUCT(PlayerStateRolling, Start, Rolling, BoostStart, Jump, StandUp, EndSquat, UnRoll,
                   Land, Brake);
}  // namespace
void PlayerStateRolling::kill() {
    mEffect->tryDeleteRollingEffect();
    al::NerveStateBase::kill();
}

bool PlayerStateRolling::update() {
    if (rs::isCollidedGround(mCollision))
        _b0 = rs::updateJudgeAndResult(mJudgeStartRise);
    return al::NerveStateBase::update();
}

bool PlayerStateRolling::isRolling() const {
    return !isDead() && !al::isNerve(this, &NrvPlayerStateRolling.Jump);
}

bool PlayerStateRolling::isRollingJump() const {
    return al::isNerve(this, &NrvPlayerStateRolling.Jump) && !isDead();
}

bool PlayerStateRolling::isEnableCancelNormalJump() const {
    return al::isNerve(this, &NrvPlayerStateRolling.StandUp);
}

bool PlayerStateRolling::isEndSquat() const {
    return al::isNerve(this, &NrvPlayerStateRolling.EndSquat);
}

bool PlayerStateRolling::isEndStandUp() const {
    return isDead() && al::isNerve(this, &NrvPlayerStateRolling.StandUp);
}

bool PlayerStateRolling::isEnableTrample(const al::HitSensor* self,
                                         const al::HitSensor* other) const {
    if (isDead() || al::isNerve(this, &NrvPlayerStateRolling.Jump))
        return true;

    const sead::Vector3f gravity = al::getGravity(mActor);
    sead::Vector3f between(0.0f, 0.0f, 0.0f);
    al::calcVecBetweenSensors(&between, self, other);
    const f32 xProjection = gravity.x * between.x;
    f32 sensorProjection = -gravity.y * between.y;
    sensorProjection -= xProjection;
    sensorProjection -= gravity.z * between.z;
    const f32 sensorRadius = al::getSensorRadius(other) + al::getSensorRadius(self);
    return !(sensorProjection < sensorRadius - (mConst->getFallSpeedMax() + 10.0f));
}

f32 PlayerStateRolling::getInverseKinematicsRate() const {
    return al::isNerve(this, &NrvPlayerStateRolling.StandUp) ? 1.0f : 0.0f;
}

bool PlayerStateRolling::isStartRollingBrake() const {
    if (!rs::isOnGroundSlopeSlideEnd(mActor, mCollision, mConst))
        return false;
    if (mSlopeSlideControl->getHorizontalVelocity().length() > mConst->getSlopeRollingSpeedEnd())
        return false;
    return _98 >= mConst->getSlopeRollingFrameMin() && !_b0;
}

void PlayerStateRolling::exeEndSquat() {
    if (al::isFirstStep(this))
        mEffect->tryDeleteRollingEffect();
}

PlayerStateRolling::~PlayerStateRolling() = default;

