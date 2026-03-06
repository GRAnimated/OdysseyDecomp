#include "Npc/TalkNpcParam.h"

#include <cstring>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Base/StringUtil.h"
#include "Library/Event/EventFlowActorParamHolder.h"
#include "Library/Event/EventFlowWatchParam.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/Joint/JointLookAtController.h"
#include "Library/Joint/JointLookAtInfo.h"
#include "Library/Joint/JointLookAtParam.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorResourceFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Yaml/ByamlIter.h"
#include "Library/Yaml/ByamlUtil.h"

#include "Npc/TalkNpcEyeLineAnimParam.h"
#include "Util/NpcEventFlowUtil.h"

static void createJointLookAtParam(al::JointLookAtParam** out, const al::ModelKeeper* modelKeeper,
                                   const al::ByamlIter& iter) {
    const char* jointName = al::getByamlKeyString(iter, "JointName");
    s32 jointIndex = al::getJointIndex(modelKeeper, jointName);
    f32 lookRate = al::getByamlKeyFloat(iter, "LookRate");

    sead::Vector2f limitYawDegree = {0.0f, 0.0f};
    sead::Vector2f limitPitchDegree = {0.0f, 0.0f};
    sead::Vector3f localFrontDir = {0.0f, 0.0f, 0.0f};
    sead::Vector3f localUpDir = {0.0f, 0.0f, 0.0f};

    al::tryGetByamlV2f(&limitYawDegree, iter, "LimitYawDegree");
    al::tryGetByamlV2f(&limitPitchDegree, iter, "LimitPitchDegree");
    al::tryGetByamlV3f(&localFrontDir, iter, "LocalFrontDir");
    al::tryGetByamlV3f(&localUpDir, iter, "LocalUpDir");

    *out = new al::JointLookAtParam(jointIndex, lookRate, limitYawDegree, limitPitchDegree,
                                    localFrontDir, localUpDir);
}

TalkNpcParam::TalkNpcParam() = default;

void TalkNpcParam::init(const al::LiveActor* actor, const char* suffix) {
    mModelName = al::getModelName(actor);
    mSuffixName = suffix;

    mEventFlowActorParamHolder = new al::EventFlowActorParamHolder();
    mEventFlowWatchParam = new al::EventFlowWatchParam();
    mEyeLineAnimParam = new TalkNpcEyeLineAnimParam();
    mFacialAnimParam = new FacialAnimParam();

    auto* birdGlide = new BirdGlideParam();
    mBirdGlideParam = birdGlide;

    al::ByamlIter rootIter;
    if (!al::tryGetActorInitFileIter(&rootIter, actor, "NpcParam", mSuffixName))
        return;

    al::tryGetByamlBool(&mIsValidTurnSeparate, rootIter, "IsValidTurnSeparate");
    al::tryGetByamlBool(&mIsInvalidScare, rootIter, "IsInvalidScare");
    al::tryGetByamlBool(&mIsUpdateMaterialCode, rootIter, "IsUpdateMaterialCode");
    al::tryGetByamlBool(&mIsValidVariableFrameRateSkeletalAnim, rootIter,
                        "IsValidVariableFrameRateSkeletalAnim");
    al::tryGetByamlBool(&mIsValidShellActorReaction, rootIter, "IsValidShellActorReaction");
    al::tryGetByamlBool(&mIsValidLeapfrogReaction, rootIter, "IsValidLeapfrogReaction");
    al::tryGetByamlBool(&mIsValidExplodeReaction, rootIter, "IsValidExplodeReaction");

    mEyeLineAnimParam->init(rootIter);
    mEventFlowActorParamHolder->load(rootIter);

    al::ByamlIter partialAnimIter;
    if (al::tryGetByamlIterByKey(&partialAnimIter, rootIter, "PartialAnimParam")) {
        al::tryGetByamlBool(&mIsValidPartialAnimReaction, partialAnimIter,
                            "IsValidPartialAnimReaction");
        mPartialAnimJointName = al::getByamlKeyString(partialAnimIter, "JointName");
    }

    al::tryGetByamlS32(&mEnableReactionRestartEventStep, rootIter,
                       "EnableReactionRestartEventStep");

    al::ByamlIter jointLookAtIter;
    if (al::tryGetByamlIterByKey(&jointLookAtIter, rootIter, "JointLookAt")) {
        s32 size = jointLookAtIter.getSize();
        mJointLookAtCount = size;
        auto** params = new al::JointLookAtParam*[size];
        if (size != 0) {
            memset(params, 0, sizeof(al::JointLookAtParam*) * size);
        }
        mJointLookAtParams = params;
        for (s32 i = 0; i < mJointLookAtCount; i++) {
            al::ByamlIter entryIter;
            al::getByamlIterByIndex(&entryIter, jointLookAtIter, i);
            createJointLookAtParam(&mJointLookAtParams[i], actor->getModelKeeper(), entryIter);
        }
    }

    al::ByamlIter invalidJointIter;
    if (al::tryGetByamlIterByKey(&invalidJointIter, rootIter, "InvalidJointLookAtSklAnim")) {
        s32 size = invalidJointIter.getSize();
        mInvalidJointLookAtSklAnimCount = size;
        mInvalidJointLookAtSklAnimNames = new const char*[size];
        if (size >= 1) {
            invalidJointIter.tryGetStringByIndex(&mInvalidJointLookAtSklAnimNames[0], 0);
            for (s32 i = 1; i < mInvalidJointLookAtSklAnimCount; i++)
                invalidJointIter.tryGetStringByIndex(&mInvalidJointLookAtSklAnimNames[i], i);
        }
    }

    al::ByamlIter groundSmoothIter;
    if (al::tryGetByamlIterByKey(&groundSmoothIter, rootIter, "JointGroundSmooth")) {
        const char* jointName = nullptr;
        groundSmoothIter.tryGetStringByKey(&jointName, "JointName");
        mJointGroundSmoothName = jointName;
    }

    mEventFlowWatchParam->load(rootIter);

    al::ByamlIter trampleIter;
    if (al::tryGetByamlIterByKey(&trampleIter, rootIter, "TrampleSensorName"))
        trampleIter.tryGetStringByIndex(&mTrampleSensorName, 0);

    al::ByamlIter animPresetIter;
    if (al::tryGetByamlIterByKey(&animPresetIter, rootIter, "AnimPreset")) {
        s32 size = animPresetIter.getSize();
        mAnimPresetCount = size;
        mAnimPresets = new AnimPresetEntry*[size];
        for (s32 i = 0; i < mAnimPresetCount; i++) {
            al::ByamlIter entryIter;
            al::getByamlIterByIndex(&entryIter, animPresetIter, i);
            const char* presetName = al::getByamlKeyString(entryIter, "PresetName");
            auto* entry = new AnimPresetEntry();
            entry->presetName = presetName;
            entry->mclAnim = nullptr;
            mAnimPresets[i] = entry;
            mAnimPresets[i]->mclAnim =
                al::tryGetByamlKeyStringOrNULL(entryIter, "MclAnim");
            mAnimPresets[i]->mtpAnim =
                al::tryGetByamlKeyStringOrNULL(entryIter, "MtpAnim");
            mAnimPresets[i]->visAnim =
                al::tryGetByamlKeyStringOrNULL(entryIter, "VisAnim");
        }
    }

    al::ByamlIter facialAnimIter;
    if (al::tryGetByamlIterByKey(&facialAnimIter, rootIter, "FacialAnimParam")) {
        mFacialAnimParam->isValid = true;
        al::tryGetByamlS32(&mFacialAnimParam->interpoleStep, facialAnimIter, "InterpoleStep");
        al::ByamlIter startJointIter;
        al::getByamlIterByKey(&startJointIter, facialAnimIter, "StartJointName");
        startJointIter.tryGetStringByIndex(&mFacialAnimParam->startJointName, 0);
    }

    al::ByamlIter birdGlideIter;
    if (al::tryGetByamlIterByKey(&birdGlideIter, rootIter, "BirdGlideParam")) {
        mBirdGlideParam->isValid = true;
        mBirdGlideParam->jointName = al::getByamlKeyString(birdGlideIter, "JointName");
        al::tryGetByamlV3f(&mBirdGlideParam->localOffset, birdGlideIter, "LocalOffset");
    }
}

bool TalkNpcParam::isEqualModelName(const al::LiveActor* actor) const {
    return al::isEqualString(al::getModelName(actor), mModelName);
}

bool TalkNpcParam::isEqualSuffixName(const char* suffix) const {
    bool result = !suffix && !mSuffixName;
    if (suffix) {
        if (mSuffixName)
            return al::isEqualString(suffix, mSuffixName);
    }
    return result;
}

al::JointLookAtController*
TalkNpcParam::createAndAppendJointLookAtController(al::LiveActor* actor) const {
    al::JointLookAtController* controller =
        al::initJointLookAtController(actor, mJointLookAtCount);
    for (s32 i = 0; i < mJointLookAtCount; i++) {
        auto* info = new al::JointLookAtInfo();
        info->param = mJointLookAtParams[i];
        controller->appendJoint(info);
    }
    return controller;
}

void TalkNpcParam::createJointGroundSmoothController(al::LiveActor* actor) const {
    al::initJointGroundSmoothController(actor, mJointGroundSmoothName);
}

void TalkNpcParam::applyMaterialAnimPreset(al::LiveActor* actor,
                                           const char* presetName) const {
    for (s32 i = 0; i < mAnimPresetCount; i++) {
        if (al::isEqualString(mAnimPresets[i]->presetName, presetName)) {
            AnimPresetEntry* entry = mAnimPresets[i];
            if (entry->mclAnim)
                al::startMclAnim(actor, entry->mclAnim);
            entry = mAnimPresets[i];
            if (entry->mtpAnim)
                al::startMtpAnim(actor, entry->mtpAnim);
            entry = mAnimPresets[i];
            if (entry->visAnim)
                al::startVisAnim(actor, entry->visAnim);
            return;
        }
    }
}

void* TalkNpcParam::getEventFlowActorParam() const {
    return mEventFlowActorParamHolder->mParam;
}

void* TalkNpcParam::findEventFlowActorSuffixParam(const char* suffix) const {
    return mEventFlowActorParamHolder->findSuffixParam(suffix);
}

bool TalkNpcParam::tryInitPartialSklAnim(al::LiveActor* actor) const {
    bool hasPartialAnim = mIsValidPartialAnimReaction;
    s32 jointAmount = 0;
    if (hasPartialAnim) {
        s32 amount = al::calcJoitsAmountFromJoint(actor, mPartialAnimJointName);
        jointAmount = amount >= 0 ? amount : 0;
        hasPartialAnim = true;
    }

    FacialAnimParam* facialParam = mFacialAnimParam;
    if (facialParam->isValid) {
        s32 amount = al::calcJoitsAmountFromJoint(actor, facialParam->startJointName);
        if (jointAmount <= amount)
            jointAmount = amount;
    } else if (!hasPartialAnim) {
        return false;
    }

    al::initPartialSklAnim(actor, 2, 2, jointAmount);
    if (mIsValidPartialAnimReaction)
        al::addPartialSklAnimPartsListRecursive(actor, mPartialAnimJointName, 0);

    facialParam = mFacialAnimParam;
    if (facialParam->isValid)
        al::addPartialSklAnimPartsListRecursive(actor, facialParam->startJointName, 1);

    return true;
}

bool TalkNpcParam::isValidFacialAnim() const {
    return mFacialAnimParam->isValid;
}

void TalkNpcParam::updateFacialAnim(al::LiveActor* actor) const {
    if (rs::isSuccessNpcEventBalloonMessage(actor) &&
        rs::isPlayingNpcEventBalloonMessageVoice(actor)) {
        if (!al::isPartialSklAnimAttached(actor, 1))
            al::startPartialSklAnimWithInterpolate(actor, "Talk", 1, 1,
                                                   mFacialAnimParam->interpoleStep, nullptr);
        else if (!al::isPartialSklAnimPlaying(actor, "Talk", 1))
            al::startPartialSklAnimWithInterpolate(actor, "Talk", 1, 1,
                                                   mFacialAnimParam->interpoleStep, nullptr);
    } else {
        if (al::isPartialSklAnimAttached(actor, 1))
            al::clearPartialSklAnimWithInterpolate(actor, 1, mFacialAnimParam->interpoleStep);
    }
}

const char* TalkNpcParam::tryGetByeByeBaseJointName(const al::LiveActor* actor) const {
    if (mJointLookAtParams && mJointLookAtCount >= 1)
        return al::getJointName(actor->getModelKeeper(), mJointLookAtParams[0]->mJointIndex);
    return nullptr;
}

void TalkNpcParam::getByeByeLocalAxisFront(sead::Vector3f* out) const {
    memcpy(out, &mJointLookAtParams[0]->mLocalFrontDir, sizeof(sead::Vector3f));
}

// NON_MATCHING: FMUL operand order and FP instruction scheduling differences
void TalkNpcParam::calcBirdGlideMtx(sead::Matrix34f* out, const al::LiveActor* actor) const {
    const sead::Matrix34f* jointMtx = al::getJointMtxPtr(actor, mBirdGlideParam->jointName);
    *out = *jointMtx;
    if (!al::isNearZero(mBirdGlideParam->localOffset, 0.001f)) {
        const sead::Vector3f& offset = mBirdGlideParam->localOffset;
        f32 x = offset.x;
        f32 y = offset.y;
        f32 z = offset.z;
        out->m[0][3] += out->m[0][0] * x + out->m[0][1] * y + out->m[0][2] * z;
        out->m[1][3] += out->m[1][0] * x + out->m[1][1] * y + out->m[1][2] * z;
        out->m[2][3] += out->m[2][0] * x + out->m[2][1] * y + out->m[2][2] * z;
    }
}

bool TalkNpcParam::isInvalidJointLookSklAnim(const char* animName) const {
    for (s32 i = 0; i < mInvalidJointLookAtSklAnimCount; i++) {
        if (al::isEqualString(animName, mInvalidJointLookAtSklAnimNames[i]))
            return true;
    }
    return false;
}

bool TalkNpcParam::isInvalidChangeAllAnimFromWait(const char* animName) const {
    if (al::isEqualString("Panic", animName))
        return true;
    return al::isEqualString("WaitPanic", animName);
}

bool TalkNpcParam::isInvalidChangeTurnAnimFromWait(const char* animName) const {
    if (al::isEqualString("Dance", animName) || al::isEqualString("Excited", animName) ||
        al::isEqualString("ExcitedLadlePan", animName) ||
        al::isEqualString("ExcitedMax", animName) ||
        al::isEqualString("ExcitedMaxJump", animName))
        return true;
    return al::isEqualString("ExcitedPan", animName);
}

bool TalkNpcParam::isPlayerWatchDisregard(const al::HitSensor* sensor) const {
    return !mEventFlowWatchParam->isWatchSensor(sensor);
}

void TalkNpcParam::calcPlayerWatchTrans(sead::Vector3f* out,
                                        const al::LiveActor* actor) const {
    mEventFlowWatchParam->calcWatchTrans(out, actor);
}

bool TalkNpcParam::isInvalidTrampleSensor(const al::HitSensor* sensor) const {
    if (mTrampleSensorName)
        return !al::isSensorName(sensor, mTrampleSensorName);
    return false;
}

bool TalkNpcParam::isEnableReactionRestartEvent(const al::IUseNerve* nerve) const {
    if (mEnableReactionRestartEventStep < 0)
        return false;
    return al::isGreaterEqualStep(nerve, mEnableReactionRestartEventStep);
}

void TalkNpcParam::manualInitLookAtJoint(const char* fileName, const char* key,
                                         const al::LiveActor* actor, const char* suffix) {
    al::ByamlIter rootIter;
    if (!al::tryGetActorInitFileIter(&rootIter, actor, fileName, suffix))
        return;

    al::ByamlIter jointLookAtIter;
    if (!al::tryGetByamlIterByKey(&jointLookAtIter, rootIter, key))
        return;

    s32 size = jointLookAtIter.getSize();
    mJointLookAtCount = size;
    auto** params = new al::JointLookAtParam*[size];
    if (size != 0) {
        memset(params, 0, sizeof(al::JointLookAtParam*) * size);
    }
    mJointLookAtParams = params;
    for (s32 i = 0; i < mJointLookAtCount; i++) {
        al::ByamlIter entryIter;
        al::getByamlIterByIndex(&entryIter, jointLookAtIter, i);
        createJointLookAtParam(&mJointLookAtParams[i], actor->getModelKeeper(), entryIter);
    }
}
