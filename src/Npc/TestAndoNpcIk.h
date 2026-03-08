#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

class CyclicCoordinateDescentIk;

class TestAndoNpcIk : public al::LiveActor {
public:
    struct MyJoint {
        const char* name = nullptr;
        sead::Quatf initLocalQ = {0, 0, 0, 1};
        sead::Quatf localQ = {0, 0, 0, 1};
        sead::Vector3f localTranslation = {0, 0, 0};
        sead::Quatf worldQ = {0, 0, 0, 1};
        sead::Vector3f worldPos = {0, 0, 0};
        MyJoint* parent = nullptr;
        MyJoint* child = nullptr;
        bool hasJointCtrl = false;
        f32 localRotX = 0;
        f32 localRotY = 0;
        f32 localRotZ = 0;
        bool enabled = true;
        sead::Vector3f minAngle = {-180, -180, -180};
        sead::Vector3f maxAngle = {180, 180, 180};
    };

    TestAndoNpcIk(const char* name);
    void init(const al::ActorInitInfo& initInfo) override;
    MyJoint* createJoint(const char* jointName);
    void linkJoint(MyJoint* parent, MyJoint* child);
    void createJointCtrl(MyJoint* joint);
    void initAfterPlacement() override;
    void calcLocalPose(MyJoint* root);
    void setInitQ(MyJoint* root);
    void control() override;
    void calcCCD(MyJoint* tip, MyJoint* root, const sead::Vector3f& target);
    void applyModel(MyJoint* root);
    MyJoint* createEmptyJoint(const char* jointName);
    void rotateJoint(MyJoint* start, MyJoint* pivot, const sead::Quatf& rot);
    void extendBone(MyJoint* root, f32 scale);
    void backLocalPose(MyJoint* root);

private:
    CyclicCoordinateDescentIk* mIk = nullptr;
    MyJoint* mLeftArmRoot = nullptr;
    MyJoint* mLeftArmTip = nullptr;
    MyJoint* mRightArmRoot = nullptr;
    MyJoint* mRightArmTip = nullptr;
    MyJoint* mLegRoot = nullptr;
    MyJoint* mLegTip = nullptr;
    u32 mFrameCounter = 0;
};

static_assert(sizeof(TestAndoNpcIk::MyJoint) == 0x90);

class SkeletonDynamics;
struct SkeletonDynamicsCallbackInfo;

class TestAndoNpcIk2 : public al::LiveActor {
public:
    TestAndoNpcIk2(const char* name);
    void init(const al::ActorInitInfo& initInfo) override;
    void callback(SkeletonDynamicsCallbackInfo* info);
    void calcAnim() override;

private:
    SkeletonDynamics* mSkeletonDynamics = nullptr;
};
