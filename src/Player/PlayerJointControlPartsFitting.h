#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Joint/JointControllerBase.h"

namespace al {
class LiveActor;
}

class PlayerJointControlPartsFitting : public al::JointControllerBase {
public:
    struct FittingInfo {
        FittingInfo();

        s32 partsJointIndex;
        sead::Vector3f initRotate;
        sead::Vector3f localTarget;
        f32 rotScale;
        sead::Vector3f transLimitMin;
        sead::Vector3f transLimitMax;
        sead::Vector3f degreeMin;
        sead::Vector3f degreeMax;
        sead::Vector3f reverseDir;
        s32 hostJointCount;
        f32 totalBlendRate;
        u8 _64[4];
        s32* hostJointIndices;
        sead::Vector3f* hostJointPositions;
        f32* blendRates;
    };

    explicit PlayerJointControlPartsFitting(const al::LiveActor*);

    void initByHostResource();
    void calcJointCallback(s32 jointIndex, sead::Matrix34f*) override;
    void calcJointFitting(sead::Matrix34f*, const FittingInfo*);
    const char* getCtrlTypeName() const override;

private:
    const al::LiveActor* mPlayer;
    al::LiveActor* mPartsActor;
    sead::PtrArray<FittingInfo> mFittingInfos;
};

static_assert(sizeof(PlayerJointControlPartsFitting::FittingInfo) == 0x80);
static_assert(sizeof(PlayerJointControlPartsFitting) == 0x48);
