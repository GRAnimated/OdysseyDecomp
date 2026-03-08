#include "Npc/GhostPlayer.h"

#include <math/seadMathCalcCommon.h>
#include <math/seadQuat.h>

#include "Library/Base/StringUtil.h"
#include "Library/Effect/EffectKeeper.h"
#include "Library/Effect/EffectSystemInfo.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/ParabolicPath.h"
#include "Library/Matrix/MatrixUtil.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Nature/WaterSurfaceFinder.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Yaml/ByamlIter.h"

#include "Npc/RaceManShell.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "Util/AreaUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(GhostPlayer, Wait);
NERVE_IMPL(GhostPlayer, Play);
NERVE_IMPL(GhostPlayer, End);
NERVE_IMPL(GhostPlayer, AttachJump);
NERVE_IMPL(GhostPlayer, ResultLose);
NERVE_IMPL(GhostPlayer, ResultWin);
NERVES_MAKE_NOSTRUCT(GhostPlayer, Wait, Play, End, AttachJump, ResultLose, ResultWin);
}  // namespace

static bool tryGetByamlSubIter(al::ByamlIter* out, const al::ByamlIter* iter, s32 dataIndex,
                               const char* key, s32 subOffset) {
    al::ByamlIter dataArray;
    if (!iter->tryGetIterByKey(&dataArray, "DataArray"))
        return false;
    al::ByamlIter entry;
    if (!dataArray.tryGetIterByIndex(&entry, dataIndex))
        return false;
    al::ByamlIter sub;
    if (!entry.tryGetIterByIndex(&sub, subOffset + 10))
        return false;
    return sub.tryGetIterByKey(out, key);
}

// NON_MATCHING: member field initialization codegen (Vector3f from .rodata const vs zero stores,
// FixedSafeString init)
GhostPlayer::GhostPlayer(const char* name, s32 goalIndex, s32 color)
    : al::LiveActor(name), mGoalIndex(goalIndex), mColor(color) {}

void GhostPlayer::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, "GhostPlayer", nullptr);
    initCommon(info);
}

void GhostPlayer::initCommon(const al::ActorInitInfo& info) {
    mParabolicPath = new al::ParabolicPath();
    al::initNerve(this, &Wait, 0);
    applyGhostData(0);

    if (mColor < 0)
        mColor = GhostPlayerColor(mGoalIndex);
    al::startMtpAnim(this, mColor.text());
    al::startVisAnim(this, "RaceManWait");

    mShell = new RaceManShell("RaceManShell");
    mShell->init(info);
    al::startMtpAnim(mShell, mColor.text());
    mShell->getName();

    al::setEffectNamedMtxPtr(this, "WaterSurface", &mWaterSurfaceMtx);

    mWaterSurfaceFinder = new al::WaterSurfaceFinder(this);
    mIsInWater = al::isInWater(this);
    mPrevTrans = al::getTrans(this);

    GameDataHolderAccessor accessor(this);
    _228 = al::isEqualString(GameDataFunction::getWorldDevelopNameCurrent(accessor), "Clash");

    makeActorAlive();
}

void GhostPlayer::initWithArchiveName(const al::ActorInitInfo& info, const char* archiveName,
                                      const char* suffix) {
    al::initActorWithArchiveName(this, info, archiveName, suffix);
    initCommon(info);
}

// NON_MATCHING: massive function (4152 bytes), complex control flow with byaml data parsing,
// euler-to-matrix conversion, hack state machine, material codes
void GhostPlayer::applyGhostData(s32 step) {
    const char* actionName = "Wait";
    s32 actionFrame = 0;

    al::ByamlIter dataArray;
    if (mPlayDataIter->tryGetIterByKey(&dataArray, "DataArray")) {
        al::ByamlIter entry;
        if (dataArray.tryGetIterByIndex(&entry, step)) {
            entry.tryGetIntByIndex(&actionFrame, 6);
            al::ByamlIter actionNameArray;
            if (mPlayDataIter->tryGetIterByKey(&actionNameArray, "ActionName"))
                actionNameArray.tryGetStringByIndex(&actionName, actionFrame);
        }
    }

    if (al::isEqualString("PoleWait", actionName))
        return;

    al::getTrans(this);
    sead::Vector3f* transPtr = al::getTransPtr(this);
    {
        al::ByamlIter arr;
        if (mPlayDataIter->tryGetIterByKey(&arr, "DataArray")) {
            al::ByamlIter ent;
            if (arr.tryGetIterByIndex(&ent, step)) {
                ent.tryGetFloatByIndex(&transPtr->x, 0);
                ent.tryGetFloatByIndex(&transPtr->y, 1);
                ent.tryGetFloatByIndex(&transPtr->z, 2);
            }
        }
    }

    sead::Vector3f* rotatePtr = al::getRotatePtr(this);
    {
        al::ByamlIter arr;
        if (mPlayDataIter->tryGetIterByKey(&arr, "DataArray")) {
            al::ByamlIter ent;
            if (arr.tryGetIterByIndex(&ent, step)) {
                ent.tryGetFloatByIndex(&rotatePtr->x, 3);
                ent.tryGetFloatByIndex(&rotatePtr->y, 4);
                ent.tryGetFloatByIndex(&rotatePtr->z, 5);
            }
        }
    }

    s32 flags = 0;
    {
        al::ByamlIter arr;
        if (mPlayDataIter->tryGetIterByKey(&arr, "DataArray")) {
            al::ByamlIter ent;
            if (arr.tryGetIterByIndex(&ent, step))
                ent.tryGetIntByIndex(&flags, 8);
        }
    }

    bool isWaterIn = (flags >> 4) & 1;

    // Cap data
    al::ByamlIter capIter;
    sead::Vector3f capPos = {0, 0, 0};
    sead::Vector3f capRotate = {0, 0, 0};
    const char* capActionName = nullptr;
    if (tryGetByamlSubIter(&capIter, mPlayDataIter, step, "Cap", 0) && mCapActor) {
        capIter.tryGetFloatByIndex(&capPos.x, 0);
        capIter.tryGetFloatByIndex(&capPos.y, 1);
        capIter.tryGetFloatByIndex(&capPos.z, 2);
        al::resetPosition(mCapActor, capPos);

        capIter.tryGetFloatByIndex(&capRotate.x, 3);
        capIter.tryGetFloatByIndex(&capRotate.y, 4);
        capIter.tryGetFloatByIndex(&capRotate.z, 5);
        al::setRotate(mCapActor, capRotate);

        s32 capActionIdx = 0;
        capIter.tryGetIntByIndex(&capActionIdx, 6);
        al::ByamlIter capActionArray;
        if (mPlayDataIter->tryGetIterByKey(&capActionArray, "ActionNameCap"))
            capActionArray.tryGetStringByIndex(&capActionName, capActionIdx);

        const char* currentAction = al::getActionName(mCapActor);
        if (currentAction && al::isEqualString(currentAction, capActionName)) {
            f32 frame = 0;
            capIter.tryGetFloatByIndex(&frame, 7);
            al::setSklAnimFrame(mCapActor, frame, 0);
        } else {
            al::startAction(mCapActor, capActionName);
        }
    }

    // Effect data
    al::ByamlIter effectIter;
    if (tryGetByamlSubIter(&effectIter, mPlayDataIter, step, "Effect", 0) ||
        tryGetByamlSubIter(&effectIter, mPlayDataIter, step, "Effect", 1)) {
        sead::Vector3f effectRotate = {0, 0, 0};
        effectIter.tryGetFloatByIndex(&effectRotate.x, 3);
        effectIter.tryGetFloatByIndex(&effectRotate.y, 4);
        effectIter.tryGetFloatByIndex(&effectRotate.z, 5);

        f32 rx = sead::Mathf::deg2rad(effectRotate.x);
        f32 ry = sead::Mathf::deg2rad(effectRotate.y);
        f32 rz = sead::Mathf::deg2rad(effectRotate.z);
        f32 sinX = sead::Mathf::sin(rx);
        f32 sinY = sead::Mathf::sin(ry);
        f32 sinZ = sead::Mathf::sin(rz);
        f32 cosX = sead::Mathf::cos(rx);
        f32 cosY = sead::Mathf::cos(ry);
        f32 cosZ = sead::Mathf::cos(rz);

        mWaterSurfaceMtx(0, 3) = 0;
        mWaterSurfaceMtx(2, 2) = cosX * cosY;
        mWaterSurfaceMtx(1, 2) = -sinY;
        mWaterSurfaceMtx(0, 2) = sinZ * cosY;
        mWaterSurfaceMtx(2, 1) = sinX * cosY;
        mWaterSurfaceMtx(0, 0) = cosY * cosZ;
        mWaterSurfaceMtx(1, 3) = 0;
        mWaterSurfaceMtx(2, 3) = 0;
        mWaterSurfaceMtx(0, 1) = (sinX * sinZ) + (sinY * (cosX * cosZ));
        mWaterSurfaceMtx(2, 0) = ((sinX * sinY) * cosZ) - (sinZ * cosX);
        mWaterSurfaceMtx(1, 0) = ((sinX * sinY) * sinZ) + (cosX * cosZ);
        mWaterSurfaceMtx(1, 1) = (sinY * (sinZ * cosX)) - (sinX * cosZ);

        sead::Vector3f effectPos = {0, 0, 0};
        effectIter.tryGetFloatByIndex(&effectPos.x, 0);
        effectIter.tryGetFloatByIndex(&effectPos.y, 1);
        effectIter.tryGetFloatByIndex(&effectPos.z, 2);
        mWaterSurfaceMtx(0, 3) = effectPos.x;
        mWaterSurfaceMtx(1, 3) = effectPos.y;
        mWaterSurfaceMtx(2, 3) = effectPos.z;

        if (flags & 0x20)
            al::startHitReaction(this, "水出入");
        if (flags & 0x40)
            al::startHitReaction(this, "水出入[大]");
    }

    // Action name handling
    al::StringTmp<64> actionNameTmp(actionName);
    mActionName.copy(actionName);
    if (!al::isEqualString(actionName, mActionName.cstr())) {
        mActionName.copy(actionName);
    }

    const char* currentAction = al::getActionName(this);
    if (currentAction && al::isEqualString(currentAction, actionNameTmp.cstr())) {
        f32 frame = 0;
        {
            al::ByamlIter arr;
            if (mPlayDataIter->tryGetIterByKey(&arr, "DataArray")) {
                al::ByamlIter ent;
                if (arr.tryGetIterByIndex(&ent, step))
                    ent.tryGetFloatByIndex(&frame, 7);
            }
        }
        al::setSklAnimFrame(this, frame, 0);
        f32 delta = frame - mActionFrame;
        if (!al::isActionOneTime(this) && frame < mActionFrame)
            delta = delta + al::getActionFrameMax(this);
        al::setActionFrameRate(this, delta);
        mActionFrame = frame;
    } else {
        al::startAction(this, actionNameTmp.cstr());
        if (al::isEqualString("Move", actionNameTmp.cstr()) ||
            al::isEqualString("MoveMoon", actionNameTmp.cstr())) {
            al::setSklAnimBlendWeightFivefold(this, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
        }
        mActionFrame = 0;
    }

    // Hack data
    const char* hackName = nullptr;
    s32 hackIdx = -1;
    {
        al::ByamlIter hackIter;
        mPlayDataIter->tryGetIntByIndex(&hackIdx, 8);
        if (hackIdx >= 0) {
            al::ByamlIter hackNameArray;
            if (mPlayDataIter->tryGetIterByKey(&hackNameArray, "HackName"))
                hackNameArray.tryGetStringByIndex(&hackName, hackIdx);
        }
    }

    bool isNoAction = false;
    if (!(_228 | _229) || hackName) {
        if (capActionName) {
            if (al::isEqualString("NoAction", capActionName)) {
                isNoAction = true;
            } else {
                isNoAction = _229;
                if (!capActionName && _229)
                    isNoAction = true;
            }
        } else {
            if (!_229)
                isNoAction = false;
            else
                isNoAction = true;
        }
        _229 = isNoAction;
    } else {
        _229 = true;
        isNoAction = true;
    }

    bool hasHack = hackName != nullptr && !isNoAction;

    // Check if riding sphinx/motorcycle
    bool isRiding =
        al::isActionPlaying(this, "SphinxRideGetOn") ||
        al::isActionPlaying(this, "SphinxRideRide") ||
        al::isActionPlaying(this, "SphinxRideStop") ||
        al::isActionPlaying(this, "SphinxRideStopStart") ||
        al::isActionPlaying(this, "SphinxRideClash") ||
        al::isActionPlaying(this, "SphinxRideRunSlow") ||
        al::isActionPlaying(this, "MotorcycleWait") ||
        al::isActionPlaying(this, "MotorcycleRide") ||
        al::isActionPlaying(this, "MotorcycleRideOn") ||
        al::isActionPlaying(this, "MotorcycleRideLand") ||
        al::isActionPlaying(this, "MotorcycleRideJump") ||
        al::isActionPlaying(this, "MotorcycleRideClash");

    s32 hackStartFrame = mHackStartFrame;
    bool hackAnimStarting = false;
    bool isHackShell = false;

    if (isRiding) {
        hackAnimStarting = isRiding;
    } else {
        bool isMotorcycleStart = al::isActionPlaying(this, "MotorcycleRideRunStart");
        hackStartFrame = mHackStartFrame;
        hackAnimStarting = isMotorcycleStart;
        if ((hasHack || isMotorcycleStart)) {
            // hack start path
        } else {
            s32 hackEndFrame = mHackEndFrame;
            if (hackStartFrame >= 1)
                _1cc = 0;
            if (mIsHackActive) {
                hackEndFrame = 0;
                mHackEndFrame = 0;
            } else if (hackEndFrame < 0) {
                isHackShell = false;
                hackAnimStarting = false;
                mHackEndFrame = -1;
                goto afterHack;
            }
            f32 endFrameF = (f32)hackEndFrame;
            f32 endMax = al::getSklAnimFrameMax(this, "RaceManHackEnd");
            if (endFrameF <= endMax) {
                al::startAction(this, "RaceManHackEnd");
                al::setSklAnimFrame(this, (f32)mHackEndFrame, 0);
                al::setActionFrameRate(this, 1.0f);
                isHackShell = false;
                hackAnimStarting = false;
                mHackEndFrame = hackEndFrame + 1;
            } else {
                isHackShell = false;
                hackAnimStarting = false;
                mHackEndFrame = -1;
            }
            goto afterHack;
        }
    }

    {
        f32 startFrameF = (f32)hackStartFrame;
        f32 startMax = al::getSklAnimFrameMax(this, "RaceManHackStart");
        if (startFrameF <= startMax) {
            ++mHackStartFrame;
            al::startAction(this, "RaceManHackStart");
            al::setSklAnimFrame(this, (f32)mHackStartFrame, 0);
            al::setActionFrameRate(this, 1.0f);
        }
        isHackShell = true;
    }

afterHack:
    mIsHackActive = hasHack;

    // Determine if hack target is visible (fork/guidepost/kuribo/etc)
    bool isHackVisible = false;
    if (hackName) {
        if (al::isEqualString(hackName, "HackFork"))
            isHackVisible = true;
        else if (al::isEqualString(hackName, "Guidepost"))
            isHackVisible = true;
        else if (al::isEqualString(hackName, "Kuribo"))
            isHackVisible = true;
        else if (al::isEqualString(hackName, "KuriboWing"))
            isHackVisible = true;
        else if (al::isEqualString(hackName, "Senobi"))
            isHackVisible = true;
        else if (al::isEqualString(hackName, "Killer"))
            isHackVisible = true;
        else
            isHackVisible = al::isEqualString(hackName, "Byugo");
    }

    // Shell position
    if (mShell) {
        if (isHackVisible) {
            al::resetPosition(mShell, capPos);
            al::setRotate(mShell, capRotate);
        } else {
            al::copyPose(mShell, this);
            al::resetPosition(mShell, al::getTrans(this));
        }
    }

    // Hack interpolation
    if (_1cc <= 0) {
        // no interpolation needed
    } else {
        f32 frameF = (f32)_1cc;
        f32 hackMax = al::getSklAnimFrameMax(this, "RaceManHackStart");
        if (frameF > hackMax || hackAnimStarting) {
            // past end of hack start anim
        } else {
            if (_1cc == 0)
                _1D0 = al::getTrans(this);

            f32 t = (f32)_1cc / al::getSklAnimFrameMax(this, "RaceManHackStart");
            sead::Vector3f interpPos;
            interpPos.x = _1D0.x * (1.0f - t) + t * capPos.x;
            interpPos.y = _1D0.y * (1.0f - t) + t * capPos.y;
            interpPos.z = _1D0.z * (1.0f - t) + t * capPos.z;
            al::resetPosition(this, interpPos);

            sead::Vector3f dir;
            dir.x = capPos.x - interpPos.x;
            dir.y = capPos.y - interpPos.y;
            dir.z = capPos.z - interpPos.z;
            if (al::tryNormalizeOrZero(&dir)) {
                sead::Quatf quat;
                if (al::isParallelDirection(sead::Vector3f::ey, dir))
                    al::makeQuatUpNoSupport(&quat, sead::Vector3f::ey);
                else
                    al::makeQuatUpFront(&quat, sead::Vector3f::ey, dir);
                al::updatePoseQuat(this, quat);
            }
        }

        f32 frameF2 = (f32)_1cc;
        f32 hackMax2 = al::getSklAnimFrameMax(this, "RaceManHackStart");
        if (frameF2 <= hackMax2) {
            ++_1cc;
            al::showModelIfHide(this);
            if (mShell && al::isAlive(mShell))
                mShell->kill();
            if (!hasHack) {
                if (mCapActor)
                    al::hideModelIfShow(mCapActor);
                al::LiveActor* subActor =
                    al::tryGetSubActor(this, "レースノコノコ帽子");
                if (subActor)
                    al::showModelIfHide(subActor);
            } else {
                if (mCapActor)
                    al::showModelIfHide(mCapActor);
                al::LiveActor* subActor =
                    al::tryGetSubActor(this, "レースノコノコ帽子");
                if (subActor)
                    al::hideModelIfShow(subActor);
            }
            if (mColor == GhostPlayerColor::RaceManGold)
                al::tryDeleteEffect(this, "Gold");
            goto afterVisibility;
        }
    }

    _1cc = 0;
    if (isHackShell) {
        al::hideModelIfShow(this);
        if (mShell && al::isDead(mShell)) {
            mShell->appear();
            al::startAction(mShell, "NokonokoShell");
        }
        if (mCapActor)
            al::hideModelIfShow(mCapActor);
        al::LiveActor* subActor =
            al::tryGetSubActor(this, "レースノコノコ帽子");
        if (subActor)
            al::hideModelIfShow(subActor);
    } else if (isNoAction) {
        al::showModelIfHide(this);
        if (mShell && al::isAlive(mShell))
            mShell->kill();
        if (mCapActor)
            al::hideModelIfShow(mCapActor);
        al::LiveActor* subActor =
            al::tryGetSubActor(this, "レースノコノコ帽子");
        if (subActor)
            al::hideModelIfShow(subActor);
    } else if (!(isWaterIn & 0x18)) {
        if (!(flags & 0x18)) {
            showDefault();
        } else {
            al::showModelIfHide(this);
            if (mShell && al::isAlive(mShell))
                mShell->kill();
            if (mCapActor)
                al::showModelIfHide(mCapActor);
            al::LiveActor* subActor =
                al::tryGetSubActor(this, "レースノコノコ帽子");
            if (subActor)
                al::hideModelIfShow(subActor);
        }
    } else {
        al::showModelIfHide(this);
        if (mShell && al::isAlive(mShell))
            mShell->kill();
        if (mCapActor)
            al::hideModelIfShow(mCapActor);
        al::LiveActor* subActor =
            al::tryGetSubActor(this, "レースノコノコ帽子");
        if (!subActor)
            goto afterSubActor;
        al::showModelIfHide(subActor);
        goto afterSubActor;
    }

    if (mColor == GhostPlayerColor::RaceManGold)
        al::tryDeleteEffect(this, "Gold");

afterVisibility:
    // Hit reactions
    if (flags & 0x2)
        al::startHitReaction(this, "着地");
    if (flags & 0x4)
        al::startHitReaction(this, "着地[走り]");

    al::updateMaterialCodeWet(this, (flags >> 7) & 1);
    al::updateMaterialCodeWater(this, (flags >> 8) & 1);
    al::updateMaterialCodePuddle(this, (flags >> 9) & 1);

    // Material code
    {
        const char* materialCode = nullptr;
        s32 materialIdx = 0;
        al::ByamlIter arr;
        if (!mPlayDataIter->tryGetIterByKey(&arr, "DataArray")) {
            al::resetMaterialCode(this);
            return;
        }
        al::ByamlIter ent;
        if (!arr.tryGetIterByIndex(&ent, step)) {
            al::resetMaterialCode(this);
            return;
        }
        ent.tryGetIntByIndex(&materialIdx, 9);
        al::ByamlIter materialArray;
        if (mPlayDataIter->tryGetIterByKey(&materialArray, "MaterialCode"))
            materialArray.tryGetStringByIndex(&materialCode, materialIdx);
        if (!materialCode) {
            al::resetMaterialCode(this);
            return;
        }
        al::setMaterialCode(this, materialCode);
        if (al::isEqualSubString(materialCode, "Lava"))
            al::startHitReaction(this,
                                 "あちちジャンプ");
    }
    return;

afterSubActor:
    if (mColor == GhostPlayerColor::RaceManGold)
        al::tryDeleteEffect(this, "Gold");
    goto afterVisibility;
}

void GhostPlayer::initAfterPlacement() {
    al::LiveActor* subActor =
        al::tryGetSubActor(this, "レースノコノコ帽子");
    if (subActor)
        al::startMtpAnim(subActor, mColor.text());
    if (mCapActor)
        al::startMtpAnim(mCapActor, mColor.text());
}

bool GhostPlayer::initGhostPlayDataFromByaml(const al::ByamlIter* iter) {
    mPlayDataIter = iter;
    al::ByamlIter dataArray;
    if (mPlayDataIter->tryGetIterByKey(&dataArray, "DataArray"))
        mDataArraySize = dataArray.getSize();
    return true;
}

bool GhostPlayer::isPlayDone() const {
    return mDataArraySize <= mRaceManStep;
}

bool GhostPlayer::initGhostPlayDataFromByaml(const char* name) {
    return true;
}

void GhostPlayer::initThrowCap(al::LiveActor* cap) {
    mCapActor = cap;
}

void GhostPlayer::control() {}

void GhostPlayer::attackSensor(al::HitSensor* self, al::HitSensor* other) {}

bool GhostPlayer::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                             al::HitSensor* self) {
    if (al::isSensorName(self, "Body")) {
        bool hidden = al::isHideModel(this);
        bool disregard = al::isMsgPlayerDisregard(msg);
        if (hidden) {
            if (disregard)
                return true;
            if (rs::isMsgPlayerDisregardHomingAttack(msg))
                return true;
            if (rs::isMsgPlayerDisregardTargetMarker(msg))
                return true;
        } else if (!disregard) {
            if (rs::isMsgPlayerDisregardHomingAttack(msg))
                return true;
            if (rs::isMsgPlayerDisregardTargetMarker(msg))
                return true;
        }
    }
    return false;
}

void GhostPlayer::start() {
    if (al::isNerve(this, &Wait))
        al::setNerve(this, &Play);
}

void GhostPlayer::exeWait() {
    if (al::isFirstStep(this)) {
        al::startActionAtRandomFrame(this, "Wait");
        if (mCapActor) {
            mCapActor->appear();
            al::startAction(mCapActor, "Stay");
            al::hideModel(mCapActor);
        }
    }
}

void GhostPlayer::showDefault() {
    al::showModelIfHide(this);
    if (mShell && al::isAlive(mShell))
        mShell->kill();
    if (mCapActor)
        al::hideModelIfShow(mCapActor);
    al::LiveActor* subActor =
        al::tryGetSubActor(this, "レースノコノコ帽子");
    if (subActor)
        al::showModelIfHide(subActor);
    if (mColor == GhostPlayerColor::RaceManGold)
        al::tryEmitEffect(this, "Gold", nullptr);
}

// NON_MATCHING: water surface logic codegen differences, regalloc
void GhostPlayer::exePlay() {
    if (al::isFirstStep(this)) {
        sead::SafeString empty("");
        if (al::isEqualString(empty, mActionName))
            al::startAction(this, "Wait");
    }

    if (mDataArraySize <= mRaceManStep) {
        al::validateClipping(this);
        al::setNerve(this, &End);
        return;
    }

    applyGhostData(mRaceManStep);

    if (!al::isAlive(mShell))
        return;

    if (mWaterSurfaceFinder->isFoundSurface()) {
        sead::Vector3f front;
        al::calcFrontDir(&front, this);
        if (al::isParallelDirection(mWaterSurfaceFinder->getSurfaceNormal(), front))
            goto updateWater;
        al::makeMtxUpFrontPos(&mWaterSurfaceMtx, mWaterSurfaceFinder->getSurfaceNormal(), front,
                              mWaterSurfaceFinder->getSurfacePosition());
    } else {
        const sead::Vector3f& trans = al::getTrans(this);
        sead::Vector3f velocity;
        velocity.x = trans.x - mPrevTrans.x;
        velocity.y = trans.y - mPrevTrans.y;
        velocity.z = trans.z - mPrevTrans.z;

        sead::Vector3f surfacePos = {0, 0, 0};
        sead::Vector3f surfaceNormal = {0, 0, 0};
        if (!rs::calcFindWaterAreaSurfaceNoWaveByArrow(this, &surfacePos, &surfaceNormal,
                                                       mPrevTrans, velocity) ||
            (al::calcFrontDir(&surfaceNormal, this),
             al::isParallelDirection(surfaceNormal, surfaceNormal))) {
            const sead::Vector3f& t = al::getTrans(this);
            mWaterSurfaceMtx(0, 3) = t.x;
            mWaterSurfaceMtx(1, 3) = t.y;
            mWaterSurfaceMtx(2, 3) = t.z;
            goto updateWater;
        }
        al::makeMtxUpFrontPos(&mWaterSurfaceMtx, surfaceNormal, surfaceNormal, surfacePos);
    }

updateWater:
    {
        bool wasInWater = mIsInWater;
        bool nowInWater = al::isInWater(this);
        if (wasInWater) {
            if (!nowInWater)
                al::startHitReaction(this, "水出入");
        } else {
            if (nowInWater)
                al::startHitReaction(this, "水出入");
        }
    }
    mIsInWater = al::isInWater(this);
    mPrevTrans = al::getTrans(this);
}

// NON_MATCHING: regalloc, stack frame size
void GhostPlayer::attachJumpToTarget(const sead::Vector3f& target) {
    const sead::Vector3f& trans = al::getTrans(this);
    const sead::Vector3f& gravity = al::getGravity(this);
    sead::Vector3f up = {-gravity.x, -gravity.y, -gravity.z};
    mParabolicPath->initFromUpVector(trans, target, up, 140.0f);
    showDefault();

    sead::Vector3f dir = target;
    const sead::Vector3f& myTrans = al::getTrans(this);
    dir.x -= myTrans.x;
    dir.y -= myTrans.y;
    dir.z -= myTrans.z;
    if (!al::tryNormalizeOrZero(&dir))
        al::calcFrontDir(&dir, this);

    sead::Quatf quat;
    if (al::isParallelDirection(sead::Vector3f::ey, dir))
        al::makeQuatUpNoSupport(&quat, sead::Vector3f::ey);
    else
        al::makeQuatUpFront(&quat, sead::Vector3f::ey, dir);
    al::updatePoseQuat(this, quat);
    al::setNerve(this, &AttachJump);
}

void GhostPlayer::exeAttachJump() {
    if (al::isFirstStep(this))
        al::startAction(this, "RaceManJumpStart");
    f32 t = al::calcNerveRate(this, -1, 40);
    mParabolicPath->calcPosition(al::getTransPtr(this), t);
    if (al::isGreaterEqualStep(this, 40)) {
        al::startAction(this, "RaceManJumpEnd");
        al::setNerve(this, &End);
    }
}

// NON_MATCHING: csel condition direction (eq vs ne)
void GhostPlayer::exeEnd() {
    if (al::isFirstStep(this))
        al::startAction(this, "TurnPoint");
    if (al::turnToTarget(this, _1a0, 5.0f))
        al::setNerve(this, _19e ? (const al::Nerve*)&ResultLose : &ResultWin);
}

// NON_MATCHING: regswap in string pointer setup for csel
void GhostPlayer::exeResult() {
    if (al::isFirstStep(this)) {
        if (_19d)
            al::startAction(this, "RaceManResultWin");
        else
            al::startAction(this, _19e ? "RaceManResultLose" : "Wait");
    }
}

void GhostPlayer::exeResultLose() {
    exeResult();
}

void GhostPlayer::exeResultWin() {
    exeResult();
}
