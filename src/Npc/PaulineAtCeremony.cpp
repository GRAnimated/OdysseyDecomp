#include "Npc/PaulineAtCeremony.h"

#include <cmath>
#include <math/seadVector.h>

#include "Library/Bgm/BgmLineFunction.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/Demo/DemoFunction.h"
#include "Library/Event/EventFlowFunction.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/Joint/JointSpringControllerHolder.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorResourceFunction.h"
#include "Library/LiveActor/ActorSensorFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Shadow/ActorShadowUtil.h"
#include "Library/Stage/StageSwitchUtil.h"
#include "Library/Thread/FunctorV0M.h"
#include "Library/Yaml/ByamlIter.h"

#include "Npc/BgmAnimeSynchronizer.h"
#include "Npc/NpcStateReaction.h"
#include "Npc/PaulineRhythmInfo.h"
#include "Npc/TalkNpcParam.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
using namespace al;
NERVE_IMPL(PaulineAtCeremony, Dance);
NERVE_IMPL(PaulineAtCeremony, StandBy);
// NON_MATCHING: exeReaction not inlined into nerve execute (compiler decision)
NERVE_IMPL(PaulineAtCeremony, Reaction);
NERVE_IMPL(PaulineAtCeremony, Intro);
NERVES_MAKE_NOSTRUCT(PaulineAtCeremony, Dance, StandBy, Reaction, Intro);
}  // namespace

static const char* sDanceAnimNames[] = {
    "DanceHat",
    "DanceA",
    "DanceB",
    "DanceC",
    "DanceHelp",
    "GoodWait",
    "GoodWait",
    "GoodWait",
    "GoodWait",
    "GoodWait",
    "GoodWait",
    "GoodWait",
    "DanceSabiA",
    "DanceOneUpGirl",
    "DanceTurn",
    "DanceBEnd",
    "DanceHelpEnd",
    "DanceAtoB",
    "DanceOneUpGirlToSabi",
};

static const char* sMouthAnimNames[] = {
    "MouthCloseSmallClose", "MouthCloseClose", "MouthCloseWideClose",
    "MouthOpenSmallClose",  "MouthOpenClose",  "MouthOpenWideClose",
    "MouthCloseSmall",      "MouthClose",      "MouthCloseWide",
    "MouthOpenSmall",       "MouthOpen",       "MouthOpenWide",
    "MouthCloseSmallLaugh", "MouthCloseLaugh", "MouthCloseWide",
    "MouthOpenSmallLaugh",  "MouthOpenLaugh",  "MouthOpenWide",
    "MouthCloseSmallEnnui", "MouthCloseEnnui", "MouthCloseWideEnnui",
    "MouthOpenSmallEnnui",  "MouthOpenEnnui",  "MouthOpenWideEnnui",
    "MouthCloseSmallShout", "MouthCloseShout", "MouthCloseWideShout",
    "MouthOpenSmallShout",  "MouthOpenShout",  "MouthOpenWideShout",
    "MouthCloseSmallWink",  "MouthCloseWink",  "MouthCloseWideWink",
    "MouthOpenSmallWink",   "MouthOpenWink",   "MouthOpenWideWink",
    "MouthIntro",           "TalkSmile",
};

static const char* sVisAnimNames[] = {
    "DefaultHat",
    "DefaultHatOff",
};

struct EyelineData {
    f32 x;
    f32 y;
};

static const EyelineData sEyelineData[] = {
    {0.0f, 0.0f},      {150.0f, 140.0f},  {0.0f, 0.0f},       {-150.0f, 140.0f},
    {150.0f, -140.0f}, {0.0f, -140.0f},   {-150.0f, -140.0f}, {150.0f, 280.0f},
    {0.0f, 280.0f},    {-150.0f, 280.0f}, {-10.0f, 140.0f},
};

static void trySwitchDepthShadowMap(al::LiveActor* actor) {
    al::IUseCamera* camera = actor;
    const sead::Vector3f& cameraPos = al::getCameraPos(camera, 0);
    const sead::Vector3f& trans = al::getTrans(actor);
    f32 dx = cameraPos.x - trans.x;
    f32 dy = cameraPos.y - trans.y;
    f32 dz = cameraPos.z - trans.z;
    f32 dist = sqrtf(dx * dx + dy * dy + dz * dz);
    if (dist >= 2500.0f) {
        if (!al::isHideDepthShadowMap(actor, 0))
            al::hideDepthShadowMap(actor, 0);
    } else {
        if (al::isHideDepthShadowMap(actor, 0))
            al::showDepthShadowMap(actor, 0);
    }
}

PaulineAtCeremony::PaulineAtCeremony(const char* name) : al::LiveActor(name) {}

// NON_MATCHING: regswap and minor instruction scheduling differences (93%)
void PaulineAtCeremony::init(const al::ActorInitInfo& info) {
    {
        s32 shadowMap = 0;
        mShadowMapFunc = rs::trySwitchDepthToSelfShadow;
        al::tryGetArg(&shadowMap, info, "ShadowMap");
        const char* suffix = u8"市長の顔";
        if (shadowMap == 1) {
            mShadowMapFunc = trySwitchDepthShadowMap;
            s32 capType = -1;
            al::tryGetArg(&capType, info, "CapType");
            if (capType >= 0)
                suffix = "HighQualityShadowMapMarioHat";
            else
                suffix = "HighQualityShadowMap";
        }
        al::initActorWithArchiveName(this, info, "CityMayorDress", suffix);
    }

    mTalkNpcParam = rs::initTalkNpcParam(this, nullptr);

    {
        s32 eventFlowSelection = 0;
        al::tryGetArg(&eventFlowSelection, info, "EventFlowSelection");
        if (eventFlowSelection == 2) {
            al::initNerve(this, &Dance, 1);
        } else {
            al::initNerve(this, &Intro, 1);
            al::listenStageSwitchOn(
                this, "SwitchStart",
                al::FunctorV0M<PaulineAtCeremony*, void (PaulineAtCeremony::*)()>(
                    this, &PaulineAtCeremony::startProcessing));
        }

        if (eventFlowSelection == 0) {
            mEventFlowExecutor = rs::initEventFlow(this, info, nullptr, nullptr);
        } else if (eventFlowSelection == 1) {
            mEventFlowExecutor =
                rs::initEventFlow(this, info, "PaulineAtCeremony", "PaulineAtCeremonyNoGrandShine");
        } else if (eventFlowSelection == 2) {
            mEventFlowExecutor =
                rs::initEventFlow(this, info, "PaulineAtCeremony", "PaulineAtShinCaptureParade");
        }
    }

    rs::initEventParam(mEventFlowExecutor, mTalkNpcParam, nullptr);
    rs::initEventCharacterName(mEventFlowExecutor, info, "PaulineAtCeremony");
    al::initEventReceiver(mEventFlowExecutor, static_cast<al::IEventFlowEventReceiver*>(this));
    rs::initEventCameraTalk(mEventFlowExecutor, info, "Default", -1.0f);
    rs::startEventFlow(mEventFlowExecutor, "Init");

    mNpcStateReaction = NpcStateReaction::createForHuman(this, nullptr);
    al::initNerveState(this, mNpcStateReaction, &Reaction, u8"リアクション");

    al::initJointControllerKeeper(this, 8);
    mNpcJointLookAtController =
        rs::tryCreateAndAppendNpcJointLookAtController(this, mTalkNpcParam, 2000.0f);

    auto* springHolder = new al::JointSpringControllerHolder();
    mJointSpringControllerHolder = springHolder;
    springHolder->init(this, "JointSpringControllerInfo");

    s32 headJointCount = al::calcJoitsAmountFromJoint(this, "Head");
    al::initPartialSklAnim(this, 1, 1, headJointCount);
    al::addPartialSklAnimPartsListRecursive(this, "Head", 0);

    {
        sead::Vector3f headPos;
        al::calcJointPos(&headPos, this, "Head");
        sead::Vector3f rootPos;
        al::calcJointPos(&rootPos, this, "AllRoot");
        mHeadHeight = fabsf(headPos.y - rootPos.y);
    }

    al::invalidateClipping(this);

    if (!al::isExistModelResourceYaml(this, "DanceAnimInfo", nullptr))
        return;
    if (!al::isExistModelResourceYaml(this, "LipSyncInfo", nullptr))
        return;
    if (!al::isExistModelResourceYaml(this, "FaceInfo", nullptr))
        return;
    if (!al::isExistModelResourceYaml(this, "EyelineInfo", nullptr))
        return;
    if (!al::isExistModelResourceYaml(this, "ReSingInfo", nullptr))
        return;

    auto* rhythmInfo =
        new PaulineRhythmInfo(this, al::getModelResourceYaml(this, "DanceAnimInfo", nullptr),
                              al::getModelResourceYaml(this, "LipSyncInfo", nullptr),
                              al::getModelResourceYaml(this, "FaceInfo", nullptr),
                              al::getModelResourceYaml(this, "EyelineInfo", nullptr),
                              al::getModelResourceYaml(this, "ReSingInfo", nullptr));
    mRhythmInfo = rhythmInfo;

    {
        al::ByamlIter iter;
        if (al::tryGetActorInitFileIter(&iter, this, "BgmRhythmSyncInfo", nullptr)) {
            mBgmAnimeSynchronizer = BgmAnimeSynchronizer::tryCreate(this, iter);
            mBgmAnimeSynchronizer->setSyncChaseRateOffsetMax(0.0f, 0.05f, 0.05f, 0.1f);
        }
    }

    {
        s32 defaultHatVisibility = 0;
        al::tryGetArg(&defaultHatVisibility, info, "DefaultHatVisibility");
        al::startVisAnim(this, sVisAnimNames[defaultHatVisibility]);
    }

    mAddDemoInfo = al::registDemoRequesterToAddDemoInfo(this, info, 0);

    makeActorAlive();

    mDemoCeremonyFrames = (s32)al::getActionFrameMax(this, "DemoCeremonyOpening");
    mJointSpringControllerHolder->offControlAll();
}

void PaulineAtCeremony::startProcessing() {
    al::setNerve(this, &StandBy);
}

void PaulineAtCeremony::control() {
    mShadowMapFunc(this);

    if (mIsSwitchDanceActionStopped)
        rs::updateNpcJointLookAtController(mNpcJointLookAtController);

    if (mRhythmInfo == nullptr)
        return;

    mRhythmInfo->update(mIsNeedSync);
    if (mRhythmInfo->_89 || mIsNeedSync) {
        mIsNeedSync = false;
        if (!forceControlForDance())
            return;
    } else {
        if (!controlForDance())
            return;
    }

    if (!mIsMuteActive && mRhythmInfo->mIsReSing) {
        mIsMuteActive = true;
        al::endBgmSituation(this, "MuteOnPaulineVocal", false);
        al::startBgmSituation(this, "MuteOffPaulineVocal", false, true);
        al::validateClipping(this);
    }

    controlForLipSync();
    controlForEyeline();

    if (mBgmAnimeSynchronizer != nullptr)
        mBgmAnimeSynchronizer->trySyncBgm();
}

bool PaulineAtCeremony::forceControlForDance() {
    s32 danceAnimId = mRhythmInfo->mDanceAnimId;
    if ((u32)danceAnimId <= 18)
        mNewDanceType = (DanceType)danceAnimId;

    s32 count = mRhythmInfo->mDanceIndex;
    if (count <= 0)
        return controlForDance();

    if (mIsSwitchDanceActionStopped ||
        (!al::isNerve(this, &Dance) && !al::isNerve(this, &StandBy) &&
         !al::isNerve(this, &Intro))) {
        if (mIsMuteActive)
            mIsMuteActive = false;
        al::endBgmSituation(this, "MuteOffPaulineVocal", false);
        al::startBgmSituation(this, "MuteOnPaulineVocal", false, true);
        return false;
    }

    s32 index = count - 1;
    s32 animId = mRhythmInfo->getDanceAnimId(index);
    mIsDanceSabi = animId > 5;
    f32 beat = mRhythmInfo->mCurrentBeat;
    f32 danceBeat = mRhythmInfo->getDanceBeat(index);
    setDanceAction((DanceType)animId, (beat - danceBeat) * 3600.0f / 204.0f);

    if ((animId & 0xFE) >= 6) {
        bool isExist = rs::isExistFaceAnim(this, sDanceAnimNames[(u8)animId]);
        mIsExistFaceAnim = isExist;
        if (isExist)
            rs::animateCityMayorFace(this, sDanceAnimNames[(u8)animId], 0.90667f);
    } else {
        mIsExistFaceAnim = false;
    }
    return true;
}

void PaulineAtCeremony::controlForReSing() {
    if (mIsMuteActive)
        return;
    if (!mRhythmInfo->mIsReSing)
        return;

    mIsMuteActive = true;
    al::endBgmSituation(this, "MuteOnPaulineVocal", false);
    al::startBgmSituation(this, "MuteOffPaulineVocal", false, true);
    al::validateClipping(this);
}

// NON_MATCHING: instruction scheduling (ldrh+shift optimization, hoisted div/mul)
void PaulineAtCeremony::controlForLipSync() {
    if (mIsMuteActive) {
        s32 lipType = mRhythmInfo->mLipType;
        s32 faceType = mRhythmInfo->mFaceType;
        if (lipType < 0) {
            if (faceType >= 0) {
                u8 mouth = mCurrentMouthType;
                if (!mIsExistFaceAnim) {
                    if (al::isNerve(this, &StandBy))
                        return;
                    s32 newMouth = mouth - 6 * (mouth / 6) + 6 * faceType;
                    rs::animateCityMayorFace(this, sMouthAnimNames[(u8)newMouth], 1.0f);
                    mCurrentMouthType = (MouthType)(u8)newMouth;
                }
            }
        } else {
            if (faceType < 0)
                faceType = mCurrentMouthType / 6;
            if (!mIsExistFaceAnim) {
                if (al::isNerve(this, &StandBy))
                    return;
                u8 newMouth = lipType + 6 * faceType;
                rs::animateCityMayorFace(this, sMouthAnimNames[newMouth], 1.0f);
                mCurrentMouthType = (MouthType)newMouth;
            }
        }
    } else if (!mIsExistFaceAnim) {
        if (al::isNerve(this, &StandBy))
            return;
        rs::animateCityMayorFace(this, "MouthClose", 1.0f);
        mCurrentMouthType = MouthClose;
    }
}

void PaulineAtCeremony::controlForEyeline() {
    s32 eyelineType = mRhythmInfo->mEyelineType;
    if ((u32)eyelineType <= 10) {
        mCurrentEyelineType = eyelineType;
        mEyelineLookAtX = sEyelineData[eyelineType].x;
        mEyelineLookAtY = sEyelineData[eyelineType].y;
    }
}

// NON_MATCHING: instruction scheduling and cbz vs cmp#1 for bool check
bool PaulineAtCeremony::controlForDance() {
    s32 danceAnimId = mRhythmInfo->mDanceAnimId;
    mIsDanceSabi = danceAnimId > 5;
    if ((u32)danceAnimId <= 18)
        mNewDanceType = (DanceType)danceAnimId;

    if (mIsSwitchDanceActionStopped ||
        (!al::isNerve(this, &Dance) && !al::isNerve(this, &StandBy) &&
         !al::isNerve(this, &Intro))) {
        if (mIsMuteActive)
            mIsMuteActive = false;
        al::startBgmSituation(this, "MuteOnPaulineVocal", false, true);
        return false;
    }

    if (mCurrentDanceType != mNewDanceType) {
        setDanceAction(mNewDanceType, -1.0f);
        if (mIsDanceSabi) {
            bool isExist = rs::isExistFaceAnim(this, sDanceAnimNames[mNewDanceType]);
            mIsExistFaceAnim = isExist;
            if (isExist) {
                f32 rate = mNewDanceType > GoodWait ? 0.90667f : 1.0f;
                rs::animateCityMayorFace(this, sDanceAnimNames[mNewDanceType], rate);
            }
        } else {
            mIsExistFaceAnim = false;
        }
    }
    return true;
}

// NON_MATCHING: if/else chain code layout (handlers grouped vs interleaved)
void PaulineAtCeremony::setDanceAction(DanceType danceType, f32 frame) {
    bool wasHairBackL = mIsHairBackLInvalidated;
    frame = frame > 0.0f ? frame : -1.0f;
    mCurrentDanceType = danceType;
    mDanceActionFrame = frame;

    if (wasHairBackL) {
        mIsHairBackLInvalidated = false;
        mJointSpringControllerHolder->tryValidateConstraints("HairBackL");
    }
    if (mIsHairBackRInvalidated) {
        mJointSpringControllerHolder->tryValidateConstraints("HairBackR");
        mIsHairBackRInvalidated = false;
    }
    if (mIsSpringControlOff) {
        mJointSpringControllerHolder->onControlAll();
        mIsSpringControlOff = false;
    }

    if (danceType <= 16) {
        if (((1 << danceType) & 6) != 0) {
            mJointSpringControllerHolder->tryInvalidateConstraints("HairBackL");
            mIsHairBackLInvalidated = true;
        } else if (((1 << danceType) & 0x2010) != 0) {
            mJointSpringControllerHolder->tryInvalidateConstraints("HairBackR");
            mIsHairBackRInvalidated = true;
        } else if (((1 << danceType) & 0x14000) != 0) {
            mJointSpringControllerHolder->offControlAll();
            mIsSpringControlOff = true;
        }
    }

    al::setNerve(this, &Dance);
}

void PaulineAtCeremony::setMouthAction(MouthType mouthType) {
    if (mIsExistFaceAnim)
        return;
    if (al::isNerve(this, &StandBy))
        return;
    rs::animateCityMayorFace(this, sMouthAnimNames[mouthType], 1.0f);
    mCurrentMouthType = mouthType;
}

void PaulineAtCeremony::setEyelineLookAtPos(f32 x, f32 y) {
    mEyelineLookAtX = x;
    mEyelineLookAtY = y;
}

void PaulineAtCeremony::calPosInFrontOfSelf(sead::Vector3f* out, f32 offsetX, f32 offsetY) {
    const sead::Vector3f& trans = al::getTrans(this);
    const sead::Vector3f& front = al::getFront(this);
    sead::Vector3f result = front * 100.0f + trans;
    result.x += offsetX;
    result.y += offsetY;
    result.z += 0.0f;
    *out = result;
}

void PaulineAtCeremony::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isSensorEye(self))
        return;
    if (rs::sendMsgPushToPlayer(other, self))
        return;
    al::sendMsgPush(other, self);
}

// NON_MATCHING: regswap and NpcStateReaction vtable offset differences
bool PaulineAtCeremony::receiveMsg(const al::SensorMsg* msg, al::HitSensor* self,
                                   al::HitSensor* other) {
    if (al::isMsgPlayerDisregard(msg)) {
        if (al::isSensorNpc(other))
            return al::isSensorName(other, "Body");
        return false;
    }

    if (rs::isMsgPlayerDisregardHomingAttack(msg))
        return true;

    if (rs::isInvalidTrampleSensor(self, mTalkNpcParam)) {
        if (!mNpcStateReaction->receiveMsgNoReaction(msg, self, other))
            return mNpcStateReaction->receiveMsg(msg, self, other);
    } else {
        if (!mNpcStateReaction->receiveMsgWithoutTrample(msg, self, other))
            return mNpcStateReaction->receiveMsg(msg, self, other);
    }

    al::invalidateClipping(this);
    al::setNerve(this, &Reaction);
    return true;
}

bool PaulineAtCeremony::receiveEvent(const al::EventFlowEventData* event) {
    if (al::isEventName(event, "StopSwitchDanceAction")) {
        mIsSwitchDanceActionStopped = true;
        if (!mIsExistFaceAnim && !al::isNerve(this, &StandBy)) {
            rs::animateCityMayorFace(this, "TalkSmile", 1.0f);
            mCurrentMouthType = TalkSmile;
        }
        mJointSpringControllerHolder->offControlAll();
        return true;
    }
    if (al::isEventName(event, "StartSwitchDanceAction")) {
        mIsSwitchDanceActionStopped = false;
        al::setNerve(this, &Dance);
        mJointSpringControllerHolder->onControlAll();
        return true;
    }
    return false;
}

void PaulineAtCeremony::endClipped() {
    mIsNeedSync = true;
    al::LiveActor::endClipped();
}

void PaulineAtCeremony::setHeadLookAtPos(f32 x, f32 y) {
    mHeadLookAtX = x;
    mHeadLookAtY = y;
}

void PaulineAtCeremony::exeStandBy() {
    if (al::isGreaterEqualStep(this, mDemoCeremonyFrames))
        al::setNerve(this, &Dance);
}

void PaulineAtCeremony::exeIntro() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "DemoCeremonyOpening");
        rs::animateCityMayorFace(this, "DemoCeremonyOpening", 1.0f);
    }
    if (al::isActionEnd(this))
        al::setNerve(this, &Dance);
    else
        rs::syncMtsAnimCityMayorFace(this);
}

// NON_MATCHING: instruction scheduling and regswap (89%)
void PaulineAtCeremony::exeDance() {
    if (al::isFirstStep(this)) {
        al::startAction(this, sDanceAnimNames[mCurrentDanceType]);
        f32 rate = 0.90667f;
        if (mCurrentDanceType <= GoodWait)
            rate = 1.0f;
        al::setActionFrameRate(this, rate);
        if (al::isActionOneTime(this))
            al::invalidateClipping(this);
        else
            al::validateClipping(this);

        if (mDanceActionFrame > 0.0f) {
            s32 frameInt = (s32)mDanceActionFrame;
            f32 frameMax = al::getActionFrameMax(this, sDanceAnimNames[mCurrentDanceType]);
            al::setActionFrame(this, (f32)(frameInt % (s32)frameMax) +
                                         (mDanceActionFrame - (f32)frameInt));
        }
    }

    if (mIsManualSetHeadLookAt) {
        rs::cancelUpdateNpcJointLookAtController(mNpcJointLookAtController);
        sead::Vector3f pos;
        calPosInFrontOfSelf(&pos, mHeadLookAtX, mHeadLookAtY);
        rs::requestLookAtTargetTrans(mNpcJointLookAtController, pos);
    }
    rs::updateNpcJointLookAtController(mNpcJointLookAtController);

    if (!mIsManualSetEyelineLookAt) {
        if (mCurrentEyelineType == 2) {
            rs::resetNpcEyeLineAnim(this);
        } else if (mCurrentEyelineType == 0) {
            rs::tryUpdateNpcEyeLineAnim(this, mTalkNpcParam);
        } else {
            sead::Vector3f eyelinePos;
            calPosInFrontOfSelf(&eyelinePos, mEyelineLookAtX, mEyelineLookAtY);
            rs::tryUpdateNpcEyeLineAnimToTarget(this, mTalkNpcParam, eyelinePos, false);
        }
    } else {
        sead::Vector3f eyelinePos;
        calPosInFrontOfSelf(&eyelinePos, mEyelineLookAtX, mEyelineLookAtY);
        rs::tryUpdateNpcEyeLineAnimToTarget(this, mTalkNpcParam, eyelinePos, false);
    }

    rs::syncMtsAnimCityMayorFace(this);
    rs::updateEventFlow(mEventFlowExecutor);
}

void PaulineAtCeremony::exeReaction() {
    if (al::isFirstStep(this)) {
        al::LiveActor* face = al::getSubActor(this, u8"市長の顔");
        al::startAction(face, "ReactionCap");
    }
    if (al::updateNerveState(this))
        al::setNerve(this, &Dance);
    else if (mTalkNpcParam->isEnableReactionRestartEvent(this))
        al::setNerve(this, &Dance);
}

void PaulineAtCeremony::setIsManualSetHeadLookAt(bool isManual) {
    mIsManualSetHeadLookAt = isManual;
}

void PaulineAtCeremony::setIsManualSetEyelineLookAt(bool isManual) {
    mIsManualSetEyelineLookAt = isManual;
}
