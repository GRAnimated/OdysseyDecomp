#include "MapObj/FastenerRailKeeper.h"

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Base/StringUtil.h"
#include "Library/Camera/CameraTicket.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitInfo.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/MapObj/FixMapParts.h"
#include "Library/Math/MathUtil.h"
#include "Library/Matrix/MatrixUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementInfo.h"
#include "Library/Rail/Rail.h"
#include "Library/Rail/RailRider.h"
#include "Library/Rail/RailUtil.h"
#include "Library/Stage/StageSwitchUtil.h"
#include "Library/Thread/FunctorV0M.h"

namespace {
NERVE_IMPL(FastenerRailKeeper, Wait)

NERVES_MAKE_NOSTRUCT(FastenerRailKeeper, Wait)
}  // namespace

FastenerRailKeeper::FastenerRailKeeper(const char* name, al::LiveActor* actor)
    : al::LiveActor(name), mFastener(static_cast<Fastener*>(actor)) {}

void FastenerRailKeeper::init(const al::ActorInitInfo& info) {
    using FastenerRailKeeperFunctor =
        al::FunctorV0M<FastenerRailKeeper*, void (FastenerRailKeeper::*)()>;

    al::initActorSceneInfo(this, info);
    al::initActorPoseTRSV(this);
    al::initActorSRT(this, info);
    al::initActorClipping(this, info);
    al::initStageSwitch(this, info);

    al::tryGetArg(&mIsShowLine, info, "IsShowLine");
    al::tryGetArg(&mIsFastener, info, "IsFastener");
    al::tryGetArg(&mIsMesh, info, "IsMesh");
    al::tryGetArg(&mIsInvalid, info, "IsInvalid");
    al::tryGetArg(&mIsUncutable, info, "IsUncutable");
    al::tryGetArg(&mIsLimitY, info, "IsLimitY");
    al::tryGetArg(&mIsFixXZ, info, "IsFixXZ");
    al::tryGetArg(&mIsMeshInvisible, info, "IsMeshInvisible");
    al::tryGetArg(&mIsSplit, info, "IsSplit");
    al::tryGetArg(&mSplitLevel, info, "SplitLevel");

    mRail = new al::Rail();
    mRail->init(*info.placementInfo);
    mRailRider = new al::RailRider(mRail);

    s32 pointNum = al::getRailPointNum(this);
    s32 needCameraPointNum = 0;
    for (s32 i = 0; i < pointNum; ++i)
        if (isRailPointIsNeedCamera(i) || isRailPointIsNeedStartCameraHackEnd(i))
            ++needCameraPointNum;
    if (needCameraPointNum > 0)
        mCameraInfos.allocBuffer(needCameraPointNum, nullptr);

    for (s32 i = 0; i < pointNum; ++i) {
        al::CameraTicket* ticket = nullptr;
        if (isRailPointIsNeedCamera(i)) {
            auto* id = new sead::FixedSafeString<0x20>();
            id->format("%d", i);
            ticket = al::initObjectCamera(mFastener, info, id->cstr(), nullptr);
        }
        al::CameraTicket* ticketEntrance = nullptr;
        if (isRailPointIsNeedStartCameraHackEnd(i)) {
            auto* id = new sead::FixedSafeString<0x20>();
            id->format("%d(Entrance)", i);
            ticketEntrance = al::initEntranceCamera(mFastener, *info.placementInfo, id->cstr());
        }
        if (ticket || ticketEntrance)
            mCameraInfos.pushBack(new RailPointCameraInfo{ticket, ticketEntrance, i});
    }

    al::initExecutorUpdate(this, info, "地形オブジェ[Movement]");
    al::initNerve(this, &Wait, 0);

    if (mIsMesh) {
        al::PlacementInfo meshPlacementInfo;
        al::ActorInitInfo meshInitInfo;
        meshInitInfo.initViewIdHost(&meshPlacementInfo, info);
        if (al::isEqualString("MeshModel", "MeshModel") &&
            al::calcLinkChildNum(info, "MeshModel")) {
            al::getLinksInfoByIndex(&meshPlacementInfo, info, "MeshModel", 0);
            mMeshModel = new al::FixMapParts("マップパーツ");
            mMeshModel->init(meshInitInfo);
            al::getModelName(mMeshModel);
        }
        s32 segmentNum = al::getRailPointNum(this) - 2;
        if (segmentNum < 0)
            segmentNum = 0;
        if (segmentNum >= 1)
            mMeshSegments.allocBuffer(segmentNum, nullptr);
    }

    makeActorAlive();

    if (al::listenStageSwitchOnAppear(
            this, FastenerRailKeeperFunctor(this, &FastenerRailKeeper::appearBySwitch)))
        kill();
    al::listenStageSwitchOnKill(this,
                                FastenerRailKeeperFunctor(this, &FastenerRailKeeper::killBySwitch));
}

void FastenerRailKeeper::appear() {
    al::LiveActor::appear();
    mFastener->tryUpdateDisplayModel();
}

void FastenerRailKeeper::kill() {
    al::LiveActor::kill();
    mFastener->tryUpdateDisplayModel();
}

al::PlacementInfo* FastenerRailKeeper::getRailPointInfo(s32 index) const {
    return al::getRailPointInfo(this, index);
}

bool FastenerRailKeeper::isRailPointIgnore(s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    bool ret;
    if (al::tryGetArg(&ret, *info, "IsIgnore"))
        return ret;
    return false;
}

bool FastenerRailKeeper::isRailPointSpringFix(s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    bool ret;
    if (al::tryGetArg(&ret, *info, "IsSpringFix"))
        return ret;
    return false;
}

bool FastenerRailKeeper::isRailPointFastenerKnob(s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    bool ret;
    if (al::tryGetArg(&ret, *info, "IsFastenerKnob"))
        return ret;
    return false;
}

bool FastenerRailKeeper::isRailPointIsNeedCamera(s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    bool ret = false;
    al::tryGetArg(&ret, *info, "IsNeedCamera");
    return ret;
}

bool FastenerRailKeeper::isRailPointCutReach(s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    bool ret;
    if (al::tryGetArg(&ret, *info, "IsCutReach"))
        return ret;
    return false;
}

bool FastenerRailKeeper::isRailPointDisappearKnob(s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    bool ret;
    if (al::tryGetArg(&ret, *info, "IsDisappearKnob"))
        return ret;
    return false;
}

bool FastenerRailKeeper::isRailPointShowCorner(s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    bool ret;
    if (al::tryGetArg(&ret, *info, "IsShowCorner"))
        return ret;
    return false;
}

bool FastenerRailKeeper::isRailPointReachOpen(s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    bool ret;
    if (al::tryGetArg(&ret, *info, "IsReachOpen"))
        return ret;
    return false;
}

bool FastenerRailKeeper::isRailPointKnobOpenOnly(s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    bool ret;
    if (al::tryGetArg(&ret, *info, "IsKnobOpenOnly"))
        return ret;
    return false;
}

bool FastenerRailKeeper::isRailPointDisableReset(s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    bool ret;
    if (al::tryGetArg(&ret, *info, "IsDisableReset"))
        return ret;
    return false;
}

bool FastenerRailKeeper::isRailPointKnobFollowPoseMeshNormal(s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    bool ret;
    if (al::tryGetArg(&ret, *info, "IsKnobFollowPoseMeshNormal"))
        return ret;
    return false;
}

bool FastenerRailKeeper::isRailPointReachCancelHack(s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    bool ret;
    if (al::tryGetArg(&ret, *info, "IsReachCancelHack"))
        return ret;
    return false;
}

bool FastenerRailKeeper::isRailPointIsNeedStartCameraHackEnd(s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    bool ret = false;
    al::tryGetArg(&ret, *info, "IsNeedStartCameraHackEnd");
    return ret;
}

bool FastenerRailKeeper::tryGetRailPointFastenerMoveLimitAreaFlag(s32* out, s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    return al::tryGetArg(out, *info, "FastenerMoveLimitAreaFlag");
}

// NON_MATCHING: instruction scheduling on second tryGetArg call
bool FastenerRailKeeper::tryGetRailPointUv(sead::Vector2f* out, s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    if (al::tryGetArg(&out->x, *info, "TexCoordU")) {
        if (al::tryGetArg(&out->y, *info, "TexCoordV"))
            return true;
    }
    return false;
}

bool FastenerRailKeeper::tryGetRailPointFront(sead::Vector3f* out, s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    return al::tryGetUp(out, *info);
}

bool FastenerRailKeeper::tryGetRailPointDestinationTrans(sead::Vector3f* out, s32 index) const {
    al::PlacementInfo* info = getRailPointInfo(index);
    return al::tryGetLinksTrans(out, *info, "DestinationPoint");
}

al::CameraTicket* FastenerRailKeeper::findRailPointCameraTicket(s32 index) const {
    for (s32 i = 0; i < mCameraInfos.size(); ++i) {
        RailPointCameraInfo* info = mCameraInfos[i];
        if (info->railPointIndex == index)
            return info->objectCamera;
    }
    return nullptr;
}

al::CameraTicket* FastenerRailKeeper::findRailPointStartCameraHackEndTicket(s32 index) const {
    for (s32 i = 0; i < mCameraInfos.size(); ++i) {
        RailPointCameraInfo* info = mCameraInfos[i];
        if (info->railPointIndex == index)
            return info->entranceCamera;
    }
    return nullptr;
}

void FastenerRailKeeper::endCameraIfActive() {
    for (s32 i = 0; i < mCameraInfos.size(); ++i) {
        RailPointCameraInfo* info = mCameraInfos[i];
        if (al::isActiveCamera(info->objectCamera))
            al::endCamera(mFastener, info->objectCamera, -1, false);
    }
}

void FastenerRailKeeper::appearBySwitch() {
    if (al::isAlive(this))
        return;
    appear();
}

void FastenerRailKeeper::killBySwitch() {
    if (al::isDead(this))
        return;
    kill();
}

void FastenerRailKeeper::exeWait() {
    al::isFirstStep(this);
}

// NON_MATCHING: regalloc in side vector calculation (ldp vs separate loads)
bool FastenerRailKeeper::calcMeshMtx(sead::Matrix34f* outMtx) {
    sead::Vector3f upAccum = sead::Vector3f::zero;
    sead::Vector3f posAccum = sead::Vector3f::zero;

    for (s32 i = 0; i < mMeshSegments.size(); ++i) {
        FaMeshSegment* seg = mMeshSegments[i];
        upAccum += seg->normal;
        posAccum += seg->knob0->pos;
        posAccum += seg->knob1->pos;
        posAccum += seg->knob2->pos;
    }

    f32 invCount = 1.0f / static_cast<f32>(3 * mMeshSegments.size());
    posAccum *= invCount;

    if (al::tryNormalizeOrZero(&upAccum)) {
        FaMeshSegment* first = mMeshSegments.size() > 0 ? mMeshSegments[0] : nullptr;

        sead::Vector3f side = first->knob1->pos - first->knob0->pos;
        if (al::tryNormalizeOrZero(&side)) {
            al::makeMtxUpSidePos(outMtx, upAccum, side, posAccum);
            return true;
        }

        sead::Vector3f side2 = first->knob2->pos - first->knob1->pos;
        if (al::tryNormalizeOrZero(&side2)) {
            al::makeMtxUpSidePos(outMtx, upAccum, side2, posAccum);
            return true;
        }
    }
    return false;
}

bool FastenerRailKeeper::calcMeshModelBaseMtx() {
    bool result = calcMeshMtx(&mMeshMtx);
    if (result)
        sead::Matrix34CalcCommon<f32>::inverse(mMeshBaseMtx, mMeshMtx);
    return result;
}

al::RailRider* FastenerRailKeeper::getRailRider() const {
    if (mRailRider)
        return mRailRider;
    return al::LiveActor::getRailRider();
}
