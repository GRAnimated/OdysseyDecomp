#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
}  // namespace al

class IUsePlayerCollision;
class CapTargetInfoFunction;

class CapTargetInfo {
public:
    CapTargetInfo();

    void init(const al::LiveActor* actor, const char* name);
    void setFollowLockOnMtx(const char* jointName, const sead::Vector3f& localTrans,
                            const sead::Vector3f& localRotate);
    void setLockOnStartAnimName(const char* animName);
    void setLockOnAnimName(const char* animName);
    void setHackName(const char* hackName);
    void makeLockOnMtx(sead::Matrix34f* outMtx) const;
    void calcLockOnFollowTargetScale(sead::Vector3f* targetScale) const;

    void setPoseMatrix(const sead::Matrix34f* mtx) { mPoseMatrix = mtx; }

    const char* getHackName() const { return mHackName; }

    const char* getLockOnAnimName() const { return mLockOnAnimName; }

    const al::LiveActor* getActor() const { return mActor; }

    bool isExistModel() const { return mIsExistModel; }

    bool get_72() const { return _72; }

    void setIsLockOnOnly() { mIsLockOnOnly = true; }

    f32 getLockOnScale() const { return mLockOnScale; }

    bool isLockOnOnly() const { return mIsLockOnOnly; }

    bool isDisableInvincibleEffect() const { return _7d; }

    bool get_7e() const { return _7e; }

    bool get_78() const { return _78; }

    bool get_7f() const { return _7f; }

    bool isUseDepthShadow() const { return mIsUseDepthShadow; }

    bool isLockOn() const { return mIsLockOn; }

    bool isInvalidCapEye() const { return mIsInvalidCapEye; }

    bool isInvalidHackThrow() const { return mIsInvalidHackThrow; }

    IUsePlayerCollision* getPlayerCollision() const { return mPlayerCollision; }

    bool isHackCancelCeilingCheck() const { return !_79; }

    bool isHackInvalidLifeRecovery() const { return _7c; }

    bool isSetHackNameToCamera() const { return mIsSetHackNameToCamera; }

    bool isEscapeLocalOffset() const { return mIsEscapeLocalOffset; }

    const sead::Vector3f& getEscapeLocalOffset() const { return mEscapeLocalOffset; }

private:
    friend CapTargetInfoFunction;
    const al::LiveActor* mActor = nullptr;
    const char* mHackName = nullptr;
    IUsePlayerCollision* mPlayerCollision = nullptr;
    const sead::Matrix34f* mPoseMatrix = nullptr;
    const sead::Matrix34f* mJointMtx = nullptr;
    sead::Vector3f mLocalTrans = sead::Vector3f::zero;
    sead::Vector3f mLocalRotate = sead::Vector3f::zero;
    f32 mLockOnScale = 1.0f;
    bool mIsUseLockOnFollowMtxScale = false;
    bool mIsUseFollowMtxScaleLocalOffset = false;
    const char* mLockOnStartAnimName = "Capture";
    const char* mLockOnAnimName = "Capture";
    bool mIsEscapeLocalOffset = false;
    sead::Vector3f mEscapeLocalOffset = {0.0f, 0.0f, 0.0f};
    const char* mName = nullptr;
    bool mIsExistModel = false;
    bool mIsLockOnOnly = false;
    bool _72 = false;
    bool mIsUseDepthShadow = false;
    bool _74 = false;
    bool mIsLockOn = true;
    bool mIsLockOnStart = false;
    bool mIsSetHackNameToCamera = false;
    bool _78 = false;
    bool _79 = false;
    bool mIsInvalidHackThrow = false;
    bool mIsInvalidCapEye = false;
    bool _7c = false;
    bool _7d = false;
    bool _7e = false;
    bool _7f = false;
};

class CapTargetInfoFunction {
public:
    static void initIterCapTargetInfo(CapTargetInfo* capTargetInfo, IUsePlayerCollision*,
                                      const al::LiveActor* actor, const char* name);
};

static_assert(sizeof(CapTargetInfo) == 0x80);
static_assert(sizeof(CapTargetInfoFunction) == 0x1);
