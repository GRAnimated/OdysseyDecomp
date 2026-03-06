#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace sead {
template <typename A1>
class IDelegate1;
}  // namespace sead

namespace al {
class LiveActor;
}  // namespace al

struct SkeletonDynamicsCallbackInfo {
    const char* name;
    sead::Vector3f pos;
    sead::Quatf quat;
};

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
    u8 _0[0x40];
};

static_assert(sizeof(SkeletonDynamics) == 0x40);
