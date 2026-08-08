#pragma once

#include <basis/seadTypes.h>
#include <container/seadSafeArray.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Joint/JointControllerBase.h"

namespace al {
class LiveActor;
}

class PlayerJointControlGroundPose : public al::JointControllerBase {
public:
    PlayerJointControlGroundPose(const al::LiveActor*);

    void calcJointCallback(s32 jointIndex, sead::Matrix34f* jointMtx) override;
    void calcTilt(s32 jointIndex, sead::Matrix34f* jointMtx);
    void calcCenterTilt(s32 jointIndex, sead::Matrix34f*);
    void calcCenterBalance(s32 jointIndex, sead::Matrix34f*);
    void calcIK(s32 jointIndex, sead::Matrix34f*);
    void calcRootMtxTilt(sead::Matrix34f* jointMtx);
    void calcSpineMtxTilt(sead::Matrix34f* jointMtx);
    void calcHeadMtxTilt(sead::Matrix34f* jointMtx);
    void calcLegTopMtxTilt(sead::Matrix34f* jointMtx, bool isRight);
    f32 initCenterBalanceRate() const;
    f32 calcCenterBalanceBlendRate() const;
    void calcRootMtx(sead::Matrix34f* jointMtx);
    void calcHipMtx(sead::Matrix34f* jointMtx);
    void calcSpineMtx(sead::Matrix34f* jointMtx);
    void calcLegBottomMtx(sead::Matrix34f*, const sead::SafeArray<sead::Matrix34f, 2>&,
                          const sead::SafeArray<s32, 2>&);
    void update(f32 tiltRate, f32 centerTiltRate, f32 centerBalanceRate, f32 ikRate,
                bool isGroundPose);
    void resetTiltRate();
    void calcGroundPoseUp(sead::Vector3f* up) const;
    void updateCenterBalance(bool, bool, bool, const sead::Vector3f&, const sead::Vector3f&,
                             bool);
    void updateCenterTilt(f32 targetRate, const sead::Vector3f& direction);
    const char* getCtrlTypeName() const override;

    f32 get_1e8() const { return _1e8; }

private:
    const al::LiveActor* mPlayer;
    s32 mJointIndexAllRoot;
    s32 mJointIndexJointRoot;
    s32 mJointIndexHip;
    s32 mJointIndexSpine1;
    s32 mJointIndexHead;
    s32 mJointIndexLegTopL;
    s32 mJointIndexLegTopR;
    s32 mJointIndexFootL;
    s32 mJointIndexFootR;
    sead::Vector3f _54;
    sead::Vector3f _60;
    sead::Vector3f _6c;
    sead::Matrix33f _78;
    sead::Matrix33f _9c;
    sead::Matrix33f _c0;
    sead::Matrix33f _e4;
    sead::SafeArray<sead::Matrix34f, 2> _108;
    sead::SafeArray<s32, 2> _168;
    sead::SafeArray<sead::Matrix34f, 2> _170;
    sead::SafeArray<s32, 2> _1d0;
    f32 _1d8;
    f32 _1dc;
    f32 _1e0;
    f32 _1e4;
    f32 _1e8;
    bool _1ec;
    u8 _1ed[3];
    sead::Vector3f _1f0;
    sead::Vector3f _1fc;
    sead::Vector3f _208;
    sead::Vector3f _214;
    f32 _220;
    bool _224;
    bool _225;
    bool _226;
    u8 _227;
    f32 _228;
    f32 _22c;
    f32 _230;
    u8 _234[4];
};

static_assert(sizeof(PlayerJointControlGroundPose) == 0x238);
