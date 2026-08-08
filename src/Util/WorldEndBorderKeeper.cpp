#include "Util/WorldEndBorderKeeper.h"

#include <math/seadMathCalcCommon.h>

#include "Library/Area/AreaObjUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
// NON_MATCHING: target/current are both 108 bytes, but the inlined Outside body schedules the Inside-nerve address load before the final member stores while current schedules it after; next source-level hypothesis is the original branch/call expression that keeps the nerve pointer live earlier.
NERVE_IMPL(WorldEndBorderKeeper, Outside)
NERVE_IMPL(WorldEndBorderKeeper, Inside)
NERVE_IMPL(WorldEndBorderKeeper, PullBack)
NERVE_IMPL(WorldEndBorderKeeper, WaitBorder)

NERVES_MAKE_NOSTRUCT(WorldEndBorderKeeper, Outside, Inside, PullBack, WaitBorder)
}  // namespace

WorldEndBorderKeeper::WorldEndBorderKeeper(const al::LiveActor* actor)
    : al::NerveExecutor("世界の果て"), mActor(actor) {
    initNerve(&Outside, 0);
}

void WorldEndBorderKeeper::reset() {
    mVelocity.set(0.0f, 0.0f, 0.0f);
    al::setNerve(this, &Outside);
}

void WorldEndBorderKeeper::update(const sead::Vector3f& position, const sead::Vector3f& velocity,
                                  bool isDemo) {
    mPosition = position;
    mInputVelocity = velocity;
    mIsDemo = isDemo;
    updateNerve();
}

// NON_MATCHING: target/current are both 108 bytes and differ only in scheduling the Inside-nerve address load relative to the final area/timer stores; next source-level hypothesis is the original call/branch source form that materializes the nerve pointer before those stores.
void WorldEndBorderKeeper::exeOutside() {
    al::AreaObj* areaObj = al::tryFindAreaObj(mActor, "WorldEndBorderArea", mPosition);
    if (!areaObj)
        return;

    mNearestEdgePos = mPosition;
    mAreaObj = areaObj;
    mBorderTimer = 0.0f;
    al::setNerve(this, &Inside);
}

// NON_MATCHING: target/current are both 428 bytes, but current keeps an extra X20 base for nextPosition and uses different min/max NaN lowering; next source-level hypothesis is the original vector-add temporary plus scalar min/max helper form that avoids the extra base register.
void WorldEndBorderKeeper::exeInside() {
    sead::Vector3f nextPosition = mPosition + mInputVelocity;
    mVelocity.set(0.0f, 0.0f, 0.0f);

    if (al::AreaObj* areaObj = al::tryFindAreaObj(mActor, "WorldEndBorderArea", nextPosition)) {
        mNearestEdgePos = mPosition;
        mAreaObj = areaObj;
        mBorderTimer = 0.0f;
        return;
    }

    f32 borderDistance = 0.0f;
    sead::Vector3f borderDirection{0.0f, 0.0f, 0.0f};
    if (!al::calcNearestAreaObjEdgePos(&mNearestEdgePos, mAreaObj, nextPosition))
        return;

    sead::Vector3f borderOffset = mNearestEdgePos - nextPosition;
    if (al::separateScalarAndDirection(&borderDistance, &borderDirection, borderOffset))
        return;

    mBorderTimer = al::converge(mBorderTimer, 60.0f, 1.0f);
    const f32 rate = sead::Mathf::sin(
        sead::Mathf::deg2rad(sead::Mathf::min(mBorderTimer / 60.0f, 1.0f) * 90.0f));
    const f32 inputDot = borderDirection.dot(mInputVelocity);
    const f32 pushSpeed = inputDot <= 0.0f ? sead::Mathf::max(-rate * inputDot, 0.0f) : 0.0f;
    mVelocity.setScale(borderDirection, pushSpeed);
    mBorderDistance = borderDistance - pushSpeed;

    if (mBorderTimer >= 60.0f)
        al::setNerve(this, &PullBack);
}

// NON_MATCHING: target/current are both 504 bytes, but current keeps an extra X20 base for nextPosition and uses different min/max NaN lowering; next source-level hypothesis is the original vector-add temporary plus scalar min/max helper form that avoids the extra base register.
void WorldEndBorderKeeper::exePullBack() {
    if (al::isFirstStep(this))
        mBorderTimer = 0.0f;

    sead::Vector3f nextPosition = mPosition + mInputVelocity;
    if (al::AreaObj* areaObj = al::tryFindAreaObj(mActor, "WorldEndBorderArea", nextPosition)) {
        mNearestEdgePos = mPosition;
        mAreaObj = areaObj;
        mBorderTimer = 0.0f;
        al::setNerve(this, &Inside);
        return;
    }

    f32 borderDistance = 0.0f;
    sead::Vector3f borderDirection{0.0f, 0.0f, 0.0f};
    if (!al::calcNearestAreaObjEdgePos(&mNearestEdgePos, mAreaObj, nextPosition))
        return;

    sead::Vector3f borderOffset = mNearestEdgePos - nextPosition;
    if (al::separateScalarAndDirection(&borderDistance, &borderDirection, borderOffset))
        return;

    mBorderTimer = al::converge(mBorderTimer, 120.0f, 1.0f);
    const f32 wave = sead::Mathf::sin(sead::Mathf::deg2rad(
        sead::Mathf::min(mBorderTimer / 120.0f, 1.0f) * 180.0f + 90.0f));
    const f32 pullDistance = al::calcRate01(wave, -1.0f, 1.0f) * mBorderDistance;
    const f32 inputDot = borderDirection.dot(mInputVelocity);

    f32 inputOpposite = 0.0f;
    f32 remainingDistance = 0.0f;
    if (inputDot <= 0.0f) {
        inputOpposite = sead::Mathf::max(-inputDot, 0.0f);
        remainingDistance =
            sead::Mathf::max((borderDistance - inputOpposite) - pullDistance, 0.0f);
    } else {
        remainingDistance = sead::Mathf::max((borderDistance - pullDistance) - inputDot, 0.0f);
    }

    mVelocity.setScale(borderDirection, inputOpposite + remainingDistance);
    if (mBorderTimer >= 120.0f)
        al::setNerve(this, &WaitBorder);
}

// NON_MATCHING: target/current are both 492 bytes with the first mismatch in the early next-position/control-flow setup; next source-level hypothesis is the original vector-add temporary lifetime and scalar min/max helper form used by the target.
void WorldEndBorderKeeper::exeWaitBorder() {
    if (al::isFirstStep(this))
        mBorderTimer = 0.0f;

    sead::Vector3f nextPosition = mPosition + mInputVelocity;
    if (al::AreaObj* areaObj = al::tryFindAreaObj(mActor, "WorldEndBorderArea", nextPosition)) {
        mNearestEdgePos = mPosition;
        mAreaObj = areaObj;
        mBorderTimer = 0.0f;
        al::setNerve(this, &Inside);
        return;
    }

    f32 borderDistance = 0.0f;
    sead::Vector3f borderDirection{0.0f, 0.0f, 0.0f};
    if (!al::calcNearestAreaObjEdgePos(&mNearestEdgePos, mAreaObj, nextPosition))
        return;

    sead::Vector3f borderOffset = mNearestEdgePos - nextPosition;
    if (al::separateScalarAndDirection(&borderDistance, &borderDirection, borderOffset))
        return;

    f32 timer = al::modf(mBorderTimer + 1.0f + 240.0f, 240.0f) + 0.0f;
    const f32 wave = sead::Mathf::cos(sead::Mathf::deg2rad(timer / 240.0f * 360.0f));
    f32 pullDistance = al::calcRate01(-wave, -1.0f, 1.0f) * 300.0f;
    const f32 pushSpeed = sead::Mathf::max(borderDistance - pullDistance, 0.0f);

    if (borderDistance < pullDistance || !mIsDemo) {
        pullDistance = sead::Mathf::min(pullDistance, borderDistance);
        const f32 angle = sead::Mathf::acos(
            al::lerpValue(-1.0f, 1.0f, sead::Mathf::min(pullDistance / 300.0f, 1.0f)));
        timer = (sead::Mathf::rad2deg(angle) + 180.0f) / 360.0f * 240.0f;
    }

    mBorderTimer = timer;
    mVelocity.setScale(borderDirection, pushSpeed);
}

WorldEndBorderKeeper::~WorldEndBorderKeeper() = default;
