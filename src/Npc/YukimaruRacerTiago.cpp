#include "Npc/YukimaruRacerTiago.h"

#include <cmath>
#include <math/seadQuat.h>
#include <math/seadVector.h>
#include <random/seadRandom.h>

#include "Library/Area/AreaObjUtil.h"
#include "Library/Base/StringUtil.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorAreaFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Rail/RailUtil.h"

#include "Npc/SnowManRaceFunction.h"
#include "Npc/YukimaruStateMove.h"
#include "System/GameDataUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(YukimaruRacerTiago, Wait);
NERVE_IMPL(YukimaruRacerTiago, Run);
NERVES_MAKE_NOSTRUCT(YukimaruRacerTiago, Wait, Run);
}  // namespace

// NON_MATCHING: sead::Quatf/Vector3f operator= generates word-sized stores instead of
// doubleword; vtable offsets differ
YukimaruRacerTiago::YukimaruRacerTiago(const char* name) : al::LiveActor(name) {
    mRotation = sead::Quatf::unit;
    mStateMove = nullptr;
    mRailErrorScale = 0.0f;
    mBaseSpeedPerLap = nullptr;
    mNumLaps = 0;
    _13c = 0;
    mIntelligence = 1.0f;
    mMoveVec = sead::Vector3f::zero;
    mFilterError = sead::Vector3f::zero;
    mFilterPrev = sead::Vector3f::zero;
    mFilterOutput = sead::Vector3f::zero;
    mUpdateInterval = 30;
    mReactionX = 0.0f;
    mReactionZ = 0.0f;
    mIdealDistanceFromPlayer = 0.0f;
    _184 = true;
}

// NON_MATCHING: vtable offset differences at appear/kill; stack addressing sp vs x29
void YukimaruRacerTiago::init(const al::ActorInitInfo& initInfo) {
    al::initActorWithArchiveName(this, initInfo, "SnowManRacer", "Racer");

    mRotation = al::getQuat(this);
    al::initJointControllerKeeper(this, 1);
    al::initJointGlobalQuatController(this, &mRotation, "Rotate");

    al::initNerve(this, &Wait, 1);

    mStateMove = new YukimaruStateMove(this, this, &mRotation);
    al::initNerveState(this, mStateMove, &Run, "レース状態");

    const char* mtpAnim = "";
    if (al::tryGetStringArg(&mtpAnim, initInfo, "MtpAnim")) {
        al::startMtpAnim(this, mtpAnim);
        al::startMclAnim(this, mtpAnim);
    }

    bool isKids = rs::isKidsMode(this);
    mNumLaps = 3;
    mBaseSpeedPerLap = new f32[3];

    const char* prefix = isKids ? "KidsMode" : "";

    for (s32 i = 0; i < 3; i++) {
        al::StringTmp<64> key("%sBaseSpeedLap%d", prefix, i + 1);
        al::tryGetArg(&mBaseSpeedPerLap[i], initInfo, key.cstr());
    }

    al::tryGetArg(&mIntelligence, initInfo, "Intelligence");
    al::tryGetArg(&mIdealDistanceFromPlayer, initInfo, "IdealDistanceFromPlayer");

    if (!al::isExistRail(initInfo, "Rail")) {
        kill();
        return;
    }

    al::setRailPosToNearestPos(this, al::getTrans(this));
    mRailErrorScale = al::getRailTotalLength(this) * 0.01;
    appear();
}

void YukimaruRacerTiago::initAfterPlacement() {
    SnowManRaceFunction::registerNpcToRaceWatcher(this);
}

void YukimaruRacerTiago::movement() {
    al::updateMaterialCodeAll(this);
    al::LiveActor::movement();
}

void YukimaruRacerTiago::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isNerve(this, &Run))
        mStateMove->attackSensor(self, other);
}

bool YukimaruRacerTiago::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                                    al::HitSensor* self) {
    if (rs::isMsgRaceStart(msg)) {
        if (al::isNerve(this, &Wait))
            al::setNerve(this, &Run);
        return true;
    }

    if (al::isNerve(this, &Run))
        mStateMove->receiveMsg(msg, other, self);
    return false;
}

void YukimaruRacerTiago::start() {
    al::setNerve(this, &Run);
}

void YukimaruRacerTiago::exeWait() {
    if (al::isFirstStep(this))
        al::startActionAtRandomFrame(this, "ReadyEnemy");
    YukimaruMovement::updateVelocity(this);
}

void YukimaruRacerTiago::exeRun() {
    updateMoveVec();
    al::updateNerveState(this);
}

// NON_MATCHING: regswap in IIR filter; branch conditions b.le vs b.ls
void YukimaruRacerTiago::updateMoveVec() {
    const sead::Vector3f& trans = al::getTrans(this);
    f32 coord = al::calcNearestRailCoord(this, trans);
    sead::Vector3f railPos;
    al::calcRailPosAtCoord(&railPos, this, coord + 900.0f);

    f32 dx = railPos.x - trans.x;
    f32 dz = railPos.z - trans.z;

    f32 errorX;
    if (fabsf(dx) <= 10.0f) {
        errorX = sead::Vector3f::zero.x;
    } else {
        f32 offset = dx > 0.0f ? -10.0f : 10.0f;
        errorX = dx + offset;
    }

    f32 errorZ;
    if (fabsf(dz) <= 10.0f) {
        errorZ = sead::Vector3f::zero.z;
    } else {
        f32 offset = dz > 0.0f ? -10.0f : 10.0f;
        errorZ = dz + offset;
    }

    f32 oldErrorX = mFilterError.x;
    mFilterError.x = errorX;
    f32 oldErrorY = mFilterError.y;
    mFilterError.y = sead::Vector3f::zero.y;
    f32 oldErrorZ = mFilterError.z;
    mFilterError.z = errorZ;

    f32 oldPrevX = mFilterPrev.x;
    f32 oldPrevY = mFilterPrev.y;
    mFilterPrev.x = oldErrorX;
    mFilterPrev.y = oldErrorY;
    f32 oldPrevZ = mFilterPrev.z;
    mFilterPrev.z = oldErrorZ;

    f32 newOutX = errorX * 3.1166f - oldErrorX * 4.4999f + oldPrevX * 1.5f;
    f32 newOutY = 0.0f * 3.1166f - oldErrorY * 4.4999f + oldPrevY * 1.5f;
    f32 newOutZ = errorZ * 3.1166f - oldErrorZ * 4.4999f + oldPrevZ * 1.5f;

    f32 prevOutX = mFilterOutput.x;
    f32 prevOutY = mFilterOutput.y;
    f32 prevOutZ = mFilterOutput.z;

    f32 corrX = newOutX + prevOutX;
    f32 corrY = newOutY + prevOutY;
    mFilterOutput.z = newOutZ;
    f32 corrZ = newOutZ + prevOutZ;
    mFilterOutput.x = newOutX;
    mFilterOutput.y = newOutY;

    sead::Vector3f correction = {corrX, corrY, corrZ};
    mMoveVec = calcMoveVector(correction);
}

// NON_MATCHING: regswap in IIR filter computation
sead::Vector3f YukimaruRacerTiago::calcCorrection(const sead::Vector3f& direction) {
    f32 oldErrorX = mFilterError.x;
    f32 oldErrorY = mFilterError.y;
    f32 oldErrorZ = mFilterError.z;

    f32 oldPrevX = mFilterPrev.x;
    f32 oldPrevY = mFilterPrev.y;
    f32 oldPrevZ = mFilterPrev.z;

    mFilterPrev.x = oldErrorX;
    mFilterPrev.y = oldErrorY;
    mFilterPrev.z = oldErrorZ;

    f32 newOutX = direction.x * 3.1166f - oldErrorX * 4.4999f + oldPrevX * 1.5f;
    f32 newOutY = direction.y * 3.1166f - oldErrorY * 4.4999f + oldPrevY * 1.5f;
    f32 newOutZ = direction.z * 3.1166f - oldErrorZ * 4.4999f + oldPrevZ * 1.5f;

    f32 prevOutX = mFilterOutput.x;
    f32 prevOutY = mFilterOutput.y;
    f32 prevOutZ = mFilterOutput.z;

    mFilterOutput.x = newOutX;
    mFilterOutput.y = newOutY;
    mFilterOutput.z = newOutZ;

    mFilterError = direction;

    return {newOutX + prevOutX, newOutY + prevOutY, newOutZ + prevOutZ};
}

// NON_MATCHING: regswap in deadzone computation and zero vector component ordering
sead::Vector3f YukimaruRacerTiago::calcError(const sead::Vector3f& pos) const {
    f32 coord = al::calcNearestRailCoord(this, pos);
    sead::Vector3f railPos;
    al::calcRailPosAtCoord(&railPos, this, coord + 900.0f);

    f32 dx = railPos.x - pos.x;
    f32 dz = railPos.z - pos.z;

    f32 zeroX = sead::Vector3f::zero.x;
    f32 zeroY = sead::Vector3f::zero.y;
    f32 zeroZ = sead::Vector3f::zero.z;

    f32 offsetX = dx > 0.0f ? -10.0f : 10.0f;
    f32 adjustedX = dx + offsetX;
    f32 errorX = fabsf(dx) > 10.0f ? adjustedX : zeroX;

    f32 offsetZ = dz > 0.0f ? -10.0f : 10.0f;
    f32 adjustedZ = dz + offsetZ;
    f32 errorZ = fabsf(dz) > 10.0f ? adjustedZ : zeroZ;

    return {errorX, zeroY, errorZ};
}

// NON_MATCHING: stack frame 0xb0 vs 0x90; (f32)(bool) generates fcsel instead of ucvtf
sead::Vector3f YukimaruRacerTiago::calcMoveVector(const sead::Vector3f& input) {
    bool isBehind;
    f32 distance;
    f32 maxPerturb = calcMaxPerturbation(&isBehind, &distance);

    sead::Random rng1;
    f32 px = maxPerturb * rng1.getF32();
    sead::Random rng2;
    f32 signX = rng2.getF32() > 0.5f ? 1.0f : -1.0f;
    px *= signX;

    rng1.init();
    f32 py = maxPerturb * rng1.getF32();
    rng2.init();
    f32 signY = rng2.getF32() > 0.5f ? 1.0f : -1.0f;
    py *= signY;

    rng1.init();
    f32 pz = maxPerturb * rng1.getF32();
    rng2.init();
    f32 signZ = rng2.getF32() > 0.5f ? 1.0f : -1.0f;
    pz *= signZ;

    sead::Vector3f perturbedInput = {px + input.x, py + input.y, input.z + pz};
    sead::Vector3f dir;
    if (!al::tryNormalizeOrZero(&dir, perturbedInput))
        dir = al::getRailDir(this);

    f32 scaledDist = distance * mRailErrorScale;
    f32 closeRatio = 0.0f;
    f32 farRatio = 0.0f;

    f32 ratio400 = scaledDist / 400.0f;
    if (ratio400 >= 0.0f) {
        closeRatio = ratio400;
        if (ratio400 > 1.0f)
            closeRatio = 1.0f;
    }

    f32 ratio750 = scaledDist / 750.0f;
    if (ratio750 >= 0.0f) {
        farRatio = ratio750;
        if (ratio750 > 1.0f)
            farRatio = 1.0f;
    }

    f32 reaction =
        closeRatio * ((f32)isBehind * 0.05f) + farRatio * ((f32)!isBehind * -0.25f);

    s32 lapNum = SnowManRaceFunction::getLapNum(this);
    s32 lapIndex = mNumLaps - 1;
    if ((u32)lapIndex >= (u32)lapNum)
        lapIndex = lapNum;

    f32 speed = mBaseSpeedPerLap[lapIndex] + reaction + mReactionZ;

    const sead::Vector3f& trans = al::getTrans(this);
    al::AreaObj* area =
        al::tryFindAreaObj(this, "YukimaruRacerMinimumSpeedEnforcementArea", trans);
    if (area) {
        f32 minSpeed = 0.0f;
        if (al::tryGetAreaObjArg(&minSpeed, area, "MinimumSpeed")) {
            if (speed < minSpeed)
                speed = minSpeed;
        }
    }

    return dir * speed;
}

// NON_MATCHING: regswap; (f32)(bool) pattern
f32 YukimaruRacerTiago::calcMaxPerturbation(bool* isBehind, f32* distance) {
    f32 myProgress = SnowManRaceFunction::calcRaceProgress(this);
    f32 playerProgress = SnowManRaceFunction::calcRaceProgressPlayer(this);

    f32 progressDiff = myProgress - playerProgress;

    if (al::isIntervalStep(this, mUpdateInterval, 0)) {
        f32 errorDist = progressDiff * mRailErrorScale;
        f32 threshold = mIdealDistanceFromPlayer;

        if (fabsf(errorDist) <= threshold) {
            mReactionX = 0.0f;
            mReactionZ = 0.0f;
        } else {
            f64 intPart;
            f32 frac = (f32)modf((f64)(errorDist / threshold), &intPart);
            f32 behind = 0.0f;
            f32 ahead = (f32)(progressDiff >= 0.0f);
            if (progressDiff < 0.0f)
                behind = 1.0f;
            f32 scaled = frac * -0.0f;
            mReactionX = (behind * 0.55f + ahead * 0.55f) * scaled;
            mReactionZ = (behind * 0.45f + ahead * 0.45f) * scaled;
        }
    }

    *isBehind = progressDiff < 0.0f;
    *distance = fabsf(progressDiff);

    f32 truncProg = truncf(myProgress) * -0.05f;
    f32 result = 0.0f;
    f32 bias = -0.0f;
    if (*isBehind)
        bias = 0.0f;
    f32 adjusted = mReactionX + (mIntelligence + truncProg * 0.0f + bias);
    if (adjusted >= 0.0f) {
        result = adjusted;
        if (adjusted > 1.0f)
            result = 1.0f;
    }
    return (1.0f - result) * 300.0f;
}

// NON_MATCHING: regswap
void YukimaruRacerTiago::calcReactionToPlayer(f32 progressDiff) {
    if (al::isIntervalStep(this, mUpdateInterval, 0)) {
        f32 errorDist = mRailErrorScale * progressDiff;
        f32 threshold = mIdealDistanceFromPlayer;

        if (fabsf(errorDist) <= threshold) {
            mReactionX = 0.0f;
            mReactionZ = 0.0f;
        } else {
            f64 intPart;
            f32 frac = (f32)modf((f64)(errorDist / threshold), &intPart);
            f32 behind = 0.0f;
            f32 ahead = (f32)(progressDiff >= 0.0f);
            if (progressDiff < 0.0f)
                behind = 1.0f;
            f32 scaled = frac * -0.0f;
            mReactionX = (behind * 0.55f + ahead * 0.55f) * scaled;
            mReactionZ = (behind * 0.45f + ahead * 0.45f) * scaled;
        }
    }
}

bool YukimaruRacerTiago::isTriggerJump() const {
    if (al::isInAreaObj(this, "YukimaruRacerNoJumpArea", al::getTrans(this)))
        return false;
    return al::isOnGround(this, 0);
}

bool YukimaruRacerTiago::isHoldJump() const {
    return al::isInAreaObj(this, "YukimaruRacerHoldJumpArea", al::getTrans(this));
}

// NON_MATCHING: sead::Vector3f operator= prevents 64-bit copy optimization
void YukimaruRacerTiago::calcInputVec(sead::Vector3f* out) const {
    *out = mMoveVec;
}

void YukimaruRacerTiago::outputInfo() {}
