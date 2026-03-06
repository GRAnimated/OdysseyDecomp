#include "Npc/SphinxRideAim.h"

#include <cmath>

#include "Library/Joint/JointAimInfo.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"

#include "Npc/SphinxRide.h"
#include "Util/PlayerUtil.h"

// NON_MATCHING: instruction scheduling of zero vector init relative to pointer loads
SphinxRideAim::SphinxRideAim(SphinxRide* sphinxRide) : mSphinxRide(sphinxRide) {
    mHeadAimInfo = new al::JointAimInfo();
    mHeadAimInfo->setPowerRate(0.0f);
    mHeadAimInfo->setInterpoleRate(0.2f);
    mHeadAimInfo->setBaseAimLocalDir(sead::Vector3f::ex);
    mHeadAimInfo->setBaseUpLocalDir(sead::Vector3f::ez);
    mHeadAimInfo->setBaseSideLocalDir(sead::Vector3f::ey);
    sead::Vector3f zero = {0.0f, 0.0f, 0.0f};
    mHeadAimInfo->setBaseOffsetLocal(zero);
    mHeadAimInfo->setLimitDegreeRect(0.0f, 0.0f, 150.0f, 150.0f);
    mHeadAimInfo->setEnableBackAim(true);
    al::initJointAimController(mSphinxRide, mHeadAimInfo, "Head");

    mNeckAimInfo = new al::JointAimInfo();
    mNeckAimInfo->setPowerRate(0.0f);
    mNeckAimInfo->setInterpoleRate(0.2f);
    mNeckAimInfo->setBaseAimLocalDir(sead::Vector3f::ex);
    mNeckAimInfo->setBaseUpLocalDir(sead::Vector3f::ez);
    mNeckAimInfo->setBaseSideLocalDir(sead::Vector3f::ey);
    zero = {0.0f, 0.0f, 0.0f};
    mNeckAimInfo->setBaseOffsetLocal(zero);
    mNeckAimInfo->setLimitDegreeRect(0.0f, 0.0f, 90.0f, 90.0f);
    mNeckAimInfo->setEnableBackAim(true);
    al::initJointAimController(mSphinxRide, mNeckAimInfo, "Neck");

    mSpineAimInfo = new al::JointAimInfo();
    mSpineAimInfo->setPowerRate(0.0f);
    mSpineAimInfo->setInterpoleRate(0.1f);
    mSpineAimInfo->setBaseAimLocalDir(sead::Vector3f::ex);
    mSpineAimInfo->setBaseUpLocalDir(sead::Vector3f::ez);
    mSpineAimInfo->setBaseSideLocalDir(sead::Vector3f::ey);
    zero = {0.0f, 0.0f, 0.0f};
    mSpineAimInfo->setBaseOffsetLocal(zero);
    mSpineAimInfo->setLimitDegreeRect(0.0f, 0.0f, 90.0f, 90.0f);
    mSpineAimInfo->setEnableBackAim(true);
    al::initJointAimController(mSphinxRide, mSpineAimInfo, "Spine2");
}

void SphinxRideAim::updateLook() {
    updateTarget();
    SphinxRide* sphinxRide = mSphinxRide;
    rs::getPlayerPos(sphinxRide);
    al::getTrans(sphinxRide);
    sead::Vector3f front;
    al::calcFrontDir(&front, sphinxRide);
    al::lerpVec(&mLookPos, mLookPos, mTargetPos, 0.03f);
    updateAimInfo();
}

// NON_MATCHING: block ordering (dot>0 case inline vs at end), fmin vs fminnm, register allocation
void SphinxRideAim::updateTarget() {
    const sead::Vector3f& playerPos = rs::getPlayerPos(mSphinxRide);
    const sead::Vector3f& trans = al::getTrans(mSphinxRide);
    f32 dx = playerPos.x - trans.x;
    f32 dy = playerPos.y - trans.y;
    f32 dz = playerPos.z - trans.z;
    f32 dist = sqrtf(dx * dx + dy * dy + dz * dz);

    sead::Vector3f dir;
    sead::Vector3f front;
    al::calcFrontDir(&front, mSphinxRide);

    if (dist > 1500.0f) {
        f32 fx = front.x * 1500.0f;
        f32 fy = front.y * 1500.0f;
        f32 fz = front.z * 1500.0f;
        const sead::Vector3f& sphinxTrans = al::getTrans(mSphinxRide);
        mTargetPos.x = fx + sphinxTrans.x;
        mTargetPos.y = fy + sphinxTrans.y;
        mTargetPos.z = fz + sphinxTrans.z;
        return;
    }

    SphinxRide* sphinxRide = mSphinxRide;
    const sead::Vector3f& pp = rs::getPlayerPos(sphinxRide);
    const sead::Vector3f& tt = al::getTrans(sphinxRide);
    f32 px = pp.x;
    f32 py = pp.y;
    f32 pz = pp.z;
    f32 tx = tt.x;
    f32 ty = tt.y;
    f32 tz = tt.z;

    const sead::Vector3f& pp2 = rs::getPlayerPos(sphinxRide);
    const sead::Vector3f& tt2 = al::getTrans(sphinxRide);
    f32 dx2 = pp2.x - tt2.x;
    f32 dy2 = pp2.y - tt2.y;
    f32 dz2 = pp2.z - tt2.z;

    al::calcFrontDir(&dir, sphinxRide);

    f32 dot = dx2 * dir.x + dy2 * dir.y + dz2 * dir.z;
    if (dot <= 0.0f) {
        f32 pdx = px - tx;
        f32 pdy = py - ty;
        f32 pdz = pz - tz;

        al::calcSideDir(&dir, sphinxRide);

        f32 sideDot = pdx * dir.x + pdy * dir.y + pdz * dir.z;
        f32 sign = sideDot < 0.0f ? -1.0f : 1.0f;

        f32 clampDist = dist < 200.0f ? dist : 200.0f;
        f32 ox = clampDist * (dir.x * sign);
        f32 oy = clampDist * (dir.y * sign);
        f32 oz = clampDist * (dir.z * sign);

        const sead::Vector3f& st = al::getTrans(sphinxRide);
        mTargetPos.x = st.x + ox;
        mTargetPos.y = st.y + oy;
        mTargetPos.z = st.z + oz;
    } else {
        const sead::Vector3f& ppos = rs::getPlayerPos(sphinxRide);
        mTargetPos.x = ppos.x;
        mTargetPos.y = ppos.y;
        mTargetPos.z = ppos.z;
    }
}

void SphinxRideAim::updateLookPos(f32 rate) {
    al::lerpVec(&mLookPos, mLookPos, mTargetPos, rate);
}

void SphinxRideAim::updateAimInfo() {
    mHeadAimInfo->setPowerRate(0.5f);
    mHeadAimInfo->setTargetPos(mLookPos);
    mNeckAimInfo->setPowerRate(0.6f);
    mNeckAimInfo->setTargetPos(mLookPos);
    mSpineAimInfo->setPowerRate(0.0f);
    mSpineAimInfo->setTargetPos(mLookPos);
}

// NON_MATCHING: register allocation (x21 for &mLookPos), pair load/store patterns
void SphinxRideAim::updateReset() {
    sead::Vector3f front;
    al::calcFrontDir(&front, mSphinxRide);
    f32 fx = front.x * 1500.0f;
    f32 fy = front.y * 1500.0f;
    f32 fz = front.z * 1500.0f;
    const sead::Vector3f& trans = al::getTrans(mSphinxRide);
    mTargetPos.x = fx + trans.x;
    mTargetPos.y = fy + trans.y;
    mTargetPos.z = fz + trans.z;
    al::lerpVec(&mLookPos, mLookPos, mTargetPos, 0.5f);
    updateAimInfo();
}
