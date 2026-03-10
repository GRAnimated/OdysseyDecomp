#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class ByamlIter;
class EventFlowActorParamHolder;
class EventFlowWatchParam;
class HitSensor;
class IUseNerve;
class JointLookAtController;
class JointLookAtParam;
class LiveActor;
}  // namespace al

class TalkNpcEyeLineAnimParam;

struct FacialAnimParam {
    FacialAnimParam() : isValid(false), startJointName(nullptr), interpoleStep(0) {}

    bool isValid;
    const char* startJointName;
    s32 interpoleStep;
};

struct BirdGlideParam {
    BirdGlideParam() : isValid(false), jointName(nullptr), localOffset(0.0f, 0.0f, 0.0f) {}

    bool isValid;
    const char* jointName;
    sead::Vector3f localOffset;
};

struct AnimPresetEntry {
    const char* presetName = nullptr;
    const char* mclAnim = nullptr;
    const char* mtpAnim = nullptr;
    const char* visAnim = nullptr;
};

class TalkNpcParam {
public:
    TalkNpcParam();

    void init(const al::LiveActor* actor, const char* suffix);
    bool isEqualModelName(const al::LiveActor* actor) const;
    bool isEqualSuffixName(const char* suffix) const;
    al::JointLookAtController* createAndAppendJointLookAtController(al::LiveActor* actor) const;
    void createJointGroundSmoothController(al::LiveActor* actor) const;
    void applyMaterialAnimPreset(al::LiveActor* actor, const char* presetName) const;
    void* getEventFlowActorParam() const;
    void* findEventFlowActorSuffixParam(const char* suffix) const;
    bool tryInitPartialSklAnim(al::LiveActor* actor) const;
    bool isValidFacialAnim() const;
    void updateFacialAnim(al::LiveActor* actor) const;
    const char* tryGetByeByeBaseJointName(const al::LiveActor* actor) const;
    void getByeByeLocalAxisFront(sead::Vector3f* out) const;
    void calcBirdGlideMtx(sead::Matrix34f* out, const al::LiveActor* actor) const;
    bool isInvalidJointLookSklAnim(const char* animName) const;
    bool isInvalidChangeAllAnimFromWait(const char* animName) const;
    bool isInvalidChangeTurnAnimFromWait(const char* animName) const;
    bool isPlayerWatchDisregard(const al::HitSensor* sensor) const;
    void calcPlayerWatchTrans(sead::Vector3f* out, const al::LiveActor* actor) const;
    bool isInvalidTrampleSensor(const al::HitSensor* sensor) const;
    bool isEnableReactionRestartEvent(const al::IUseNerve* nerve) const;
    void manualInitLookAtJoint(const char* fileName, const char* key, const al::LiveActor* actor,
                               const char* suffix);

    const char* mModelName = nullptr;
    const char* mSuffixName = nullptr;
    al::EventFlowActorParamHolder* mEventFlowActorParamHolder = nullptr;
    al::EventFlowWatchParam* mEventFlowWatchParam = nullptr;
    const char* mTrampleSensorName = nullptr;
    bool mIsValidPartialAnimReaction = false;
    bool mIsValidShellActorReaction = false;
    bool mIsValidLeapfrogReaction = false;
    bool mIsValidExplodeReaction = false;
    s32 mEnableReactionRestartEventStep = -1;
    const char* mPartialAnimJointName = nullptr;
    bool mIsValidTurnSeparate = false;
    bool mIsInvalidScare = false;
    bool mIsUpdateMaterialCode = false;
    bool mIsValidVariableFrameRateSkeletalAnim = false;
    s32 mJointLookAtCount = 0;
    al::JointLookAtParam** mJointLookAtParams = nullptr;
    s32 mInvalidJointLookAtSklAnimCount = 0;
    const char** mInvalidJointLookAtSklAnimNames = nullptr;
    const char* mJointGroundSmoothName = nullptr;
    AnimPresetEntry** mAnimPresets = nullptr;
    s32 mAnimPresetCount = 0;
    TalkNpcEyeLineAnimParam* mEyeLineAnimParam = nullptr;
    FacialAnimParam* mFacialAnimParam = nullptr;
    BirdGlideParam* mBirdGlideParam = nullptr;
};

static_assert(sizeof(TalkNpcParam) == 0x88);
