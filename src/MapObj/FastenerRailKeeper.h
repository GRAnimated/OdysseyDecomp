#pragma once

#include <container/seadPtrArray.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

#include "MapObj/Fastener.h"

namespace al {
class CameraTicket;
class FixMapParts;
class PlacementInfo;
class Rail;
class RailRider;
struct ActorInitInfo;
}  // namespace al

struct FaMeshKnob {
    u8 _0[20];
    sead::Vector3f pos;
};

struct FaMeshSegment {
    FaMeshKnob* knob0;
    FaMeshKnob* knob1;
    FaMeshKnob* knob2;
    FaMeshSegment* _18;
    sead::Vector3f normal;
};

class FastenerRailKeeper : public al::LiveActor {
public:
    struct RailPointCameraInfo {
        al::CameraTicket* objectCamera;
        al::CameraTicket* entranceCamera;
        s32 railPointIndex;
    };

    FastenerRailKeeper(const char* name, al::LiveActor* actor);

    void init(const al::ActorInitInfo& info) override;
    void appear() override;
    void kill() override;

    al::PlacementInfo* getRailPointInfo(s32 index) const;
    bool isRailPointIgnore(s32 index) const;
    bool isRailPointSpringFix(s32 index) const;
    bool isRailPointFastenerKnob(s32 index) const;
    bool isRailPointIsNeedCamera(s32 index) const;
    bool isRailPointCutReach(s32 index) const;
    bool isRailPointDisappearKnob(s32 index) const;
    bool isRailPointShowCorner(s32 index) const;
    bool isRailPointReachOpen(s32 index) const;
    bool isRailPointKnobOpenOnly(s32 index) const;
    bool isRailPointDisableReset(s32 index) const;
    bool isRailPointKnobFollowPoseMeshNormal(s32 index) const;
    bool isRailPointReachCancelHack(s32 index) const;
    bool isRailPointIsNeedStartCameraHackEnd(s32 index) const;
    bool tryGetRailPointFastenerMoveLimitAreaFlag(s32* out, s32 index) const;
    bool tryGetRailPointUv(sead::Vector2f* out, s32 index) const;
    bool tryGetRailPointFront(sead::Vector3f* out, s32 index) const;
    bool tryGetRailPointDestinationTrans(sead::Vector3f* out, s32 index) const;
    al::CameraTicket* findRailPointCameraTicket(s32 index) const;
    al::CameraTicket* findRailPointStartCameraHackEndTicket(s32 index) const;
    void endCameraIfActive();

    void appearBySwitch();
    void killBySwitch();

    void exeWait();

    bool calcMeshMtx(sead::Matrix34f* outMtx);
    bool calcMeshModelBaseMtx();

    al::RailRider* getRailRider() const override;

private:
    Fastener* mFastener = nullptr;
    al::Rail* mRail = nullptr;
    al::RailRider* mRailRider = nullptr;
    f32 mUnused = 75.0f;
    bool mIsShowLine = true;
    sead::PtrArray<RailPointCameraInfo> mCameraInfos{};
    bool mIsFastener = false;
    bool mIsMesh = false;
    bool mIsInvalid = false;
    bool mIsUncutable = false;
    bool mIsLimitY = false;
    bool mIsFixXZ = false;
    bool mIsMeshInvisible = false;
    bool mIsSplit = false;
    s32 mSplitLevel = 0;
    al::FixMapParts* mMeshModel = nullptr;
    sead::PtrArray<FaMeshSegment> mMeshSegments{};
    sead::Matrix34f mMeshMtx = sead::Matrix34f::ident;
    sead::Matrix34f mMeshBaseMtx = sead::Matrix34f::ident;
};

static_assert(sizeof(FastenerRailKeeper) == 0x1C0);
