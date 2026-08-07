#include "Player/HackCapJointControlKeeper.h"

#include "Library/Joint/JointControllerKeeper.h"
#include "Library/Math/MathUtil.h"

HackCapJointControlKeeper::HackCapJointControlKeeper()
    : mRotateY(0.0f), mRotateYVelocity(0.0f), mRotateZ(0.0f), mRotateZVelocity(0.0f) {}

void HackCapJointControlKeeper::initCapJointControl(al::LiveActor* actor) {
    al::initJointControllerKeeper(actor, 3);
    al::initJointLocalXRotator(actor, &mRotateY, "AllRoot");
    al::initJointLocalZRotator(actor, &mRotateZ, "JointRoot");
    al::initJointLocalYRotator(actor, &mRotateYVelocity, "JointRoot");
}

void HackCapJointControlKeeper::initDisplayCapJointControl(al::LiveActor* actor) {
    al::initJointControllerKeeper(actor, 2);
    al::initJointLocalZRotator(actor, &mRotateZ, "JointRoot");
    al::initJointLocalYRotator(actor, &mRotateYVelocity, "JointRoot");
}

void HackCapJointControlKeeper::updateRotateThrowZ(f32 target, f32 spring) {
    mRotateZVelocity +=
        al::calcSpringDumperForce(mRotateZ, mRotateZVelocity, target, spring);
    mRotateZ += mRotateZVelocity;
}

void HackCapJointControlKeeper::updateRotateStayZ(f32 rate) {
    const f32 target = al::lerpValue(mRotateZ, 0.0f, rate);
    mRotateZVelocity = al::lerpValue(mRotateZVelocity, target - mRotateZ, rate);
    mRotateZ += mRotateZVelocity;
}

void HackCapJointControlKeeper::updateRotateY(f32 rate) {
    mRotateYVelocity = al::modf(mRotateYVelocity - rate + 360.0f, 360.0f) + 0.0f;
}
