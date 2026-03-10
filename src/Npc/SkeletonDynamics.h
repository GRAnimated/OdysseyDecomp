#pragma once

#include <container/seadPtrArray.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace sead {
template <typename A1>
class IDelegate1;
}  // namespace sead

namespace al {
class LiveActor;
class ModelKeeper;
}  // namespace al

struct SkeletonDynamicsCallbackInfo {
    const char* name;
    sead::Vector3f pos;
    sead::Quatf quat;
};

struct JointNode;
struct SpringLink;

class SkeletonDynamics {
public:
    SkeletonDynamics(al::LiveActor* actor);
    void setAttrCtrl(const char* boneName);
    void setAttrConstraint(const char* boneName);
    void setAttrNoCtrl(const char* boneName);
    void setAttrAuto(const char* boneName, const sead::Vector3f& gravity);
    void setDelegate(sead::IDelegate1<SkeletonDynamicsCallbackInfo*>* delegate);
    void init();
    void update();

private:
    void updateAnimPose();
    void calcAnimPoseInvRot();
    void updateAnimPoseLocal();
    void updateSpring();
    void invokeDelegate();
    void updateConstrainedJoint();
    void clearForceAndTorque();
    void updateForceAndTorque();
    void updatePosAndRot();
    void clearLengthConstraint();
    f32 updateLengthConstraint();
    void applyLengthConstraint();
    void updateAnimJoint();
    void updateAutoJoint();
    void applyModel();

    al::LiveActor* mActor;
    sead::PtrArray<JointNode> mAllJoints;
    sead::PtrArray<JointNode> mCtrlJoints;
    sead::PtrArray<SpringLink> mLinks;
    sead::IDelegate1<SkeletonDynamicsCallbackInfo*>* mDelegate = nullptr;
};

static_assert(sizeof(SkeletonDynamics) == 0x40);
