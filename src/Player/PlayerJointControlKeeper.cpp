#include "Player/PlayerJointControlKeeper.h"

void PlayerJointControlKeeper::resetPartsDynamics() {
    mPartsDynamics->resetDynamics();
}

void PlayerJointControlKeeper::calcGroundPoseUp(sead::Vector3f* out) const {
    mGroundPose->calcGroundPoseUp(out);
}

// NON_MATCHING: target uses one LDP and two unaligned STURs (0x18); current aggregate copy emits two LDRs (0x1c). Next hypothesis: a 4-byte-aligned two-qword parameter aggregate.
void PlayerJointControlKeeper::updateSwimPose() {
    mSwimPose->setParam(*mSwimParam);
}

void PlayerJointControlKeeper::updatePoleClimb() {
    mPoleClimbPose->update(mHandLegAngleParam->handAngle, mHandLegAngleParam->legAngle,
                           mHandLegAngleParam->blendRate);
}

void PlayerJointControlKeeper::updatePartsDynamics() {
    mPartsDynamics->update(mCapDynamicsRate, mNoseDynamicsRate);
}

f32 PlayerJointControlKeeper::getHeadTiltRate() const {
    return mGroundPose->get_1e8();
}
