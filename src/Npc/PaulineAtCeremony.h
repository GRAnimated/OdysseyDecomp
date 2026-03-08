#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/LiveActor/LiveActor.h"

namespace al {
class AddDemoInfo;
class EventFlowEventData;
class EventFlowExecutor;
class JointSpringControllerHolder;
struct ActorInitInfo;
}  // namespace al

class BgmAnimeSynchronizer;
class NpcJointLookAtController;
class NpcStateReaction;
class PaulineRhythmInfo;
class TalkNpcParam;

using PaulineAtCeremonyShadowFunc = void (*)(al::LiveActor*);

class PaulineAtCeremony : public al::LiveActor, public al::IEventFlowEventReceiver {
public:
    enum DanceType : u8 {
        DanceHat = 0,
        DanceA = 1,
        DanceB = 2,
        DanceC = 3,
        DanceHelp = 4,
        GoodWait = 5,
        GoodWait2 = 6,
        GoodWait3 = 7,
        GoodWait4 = 8,
        GoodWait5 = 9,
        GoodWait6 = 10,
        GoodWait7 = 11,
        DanceSabiA = 12,
        DanceOneUpGirl = 13,
        DanceTurn = 14,
        DanceBEnd = 15,
        DanceHelpEnd = 16,
        DanceAtoB = 17,
        DanceOneUpGirlToSabi = 18,
    };

    enum MouthType : u8 {
        MouthCloseSmallClose = 0,
        MouthCloseClose = 1,
        MouthCloseWideClose = 2,
        MouthOpenSmallClose = 3,
        MouthOpenClose = 4,
        MouthOpenWideClose = 5,
        MouthCloseSmall = 6,
        MouthClose = 7,
        MouthCloseWide = 8,
        MouthOpenSmall = 9,
        MouthOpen = 10,
        MouthOpenWide = 11,
        MouthCloseSmallLaugh = 12,
        MouthCloseLaugh = 13,
        MouthCloseWideLaugh = 14,
        MouthOpenSmallLaugh = 15,
        MouthOpenLaugh = 16,
        MouthOpenWideLaugh = 17,
        MouthCloseSmallEnnui = 18,
        MouthCloseEnnui = 19,
        MouthCloseWideEnnui = 20,
        MouthOpenSmallEnnui = 21,
        MouthOpenEnnui = 22,
        MouthOpenWideEnnui = 23,
        MouthCloseSmallShout = 24,
        MouthCloseShout = 25,
        MouthCloseWideShout = 26,
        MouthOpenSmallShout = 27,
        MouthOpenShout = 28,
        MouthOpenWideShout = 29,
        MouthCloseSmallWink = 30,
        MouthCloseWink = 31,
        MouthCloseWideWink = 32,
        MouthOpenSmallWink = 33,
        MouthOpenWink = 34,
        MouthOpenWideWink = 35,
        MouthIntro = 36,
        TalkSmile = 37,
    };

    PaulineAtCeremony(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void startProcessing();
    void control() override;
    bool forceControlForDance();
    void controlForReSing();
    void controlForLipSync();
    void controlForEyeline();
    bool controlForDance();
    void setDanceAction(DanceType danceType, f32 frame);
    void setMouthAction(MouthType mouthType);
    void setEyelineLookAtPos(f32 x, f32 y);
    void calPosInFrontOfSelf(sead::Vector3f* out, f32 offsetX, f32 offsetY);
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg*, al::HitSensor* self, al::HitSensor* other) override;
    bool receiveEvent(const al::EventFlowEventData* event) override;
    void endClipped() override;
    void setHeadLookAtPos(f32 x, f32 y);
    void exeStandBy();
    void exeIntro();
    void exeDance();
    void exeReaction();
    void setIsManualSetHeadLookAt(bool isManual);
    void setIsManualSetEyelineLookAt(bool isManual);

private:
    TalkNpcParam* mTalkNpcParam = nullptr;
    NpcStateReaction* mNpcStateReaction = nullptr;
    NpcJointLookAtController* mNpcJointLookAtController = nullptr;
    bool mIsManualSetHeadLookAt = false;
    bool mIsSwitchDanceActionStopped = false;
    bool mIsMuteActive = true;
    bool mIsExistFaceAnim = false;
    bool mIsDanceSabi = false;
    MouthType mCurrentMouthType = MouthClose;
    u8 mCurrentEyelineType = 0;
    DanceType mCurrentDanceType = GoodWait;
    DanceType mNewDanceType = GoodWait;
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    PaulineRhythmInfo* mRhythmInfo = nullptr;
    BgmAnimeSynchronizer* mBgmAnimeSynchronizer = nullptr;
    al::JointSpringControllerHolder* mJointSpringControllerHolder = nullptr;
    bool mIsManualSetEyelineLookAt = false;
    f32 mHeadHeight = 0.0f;
    f32 mEyelineLookAtX = 0.0f;
    f32 mEyelineLookAtY = 0.0f;
    f32 mHeadLookAtX = 0.0f;
    f32 mHeadLookAtY = 0.0f;
    al::AddDemoInfo* mAddDemoInfo = nullptr;
    s32 mDemoCeremonyFrames = 0;
    f32 mDanceActionFrame = -1.0f;
    bool mIsNeedSync = false;
    bool mIsHairBackLInvalidated = false;
    bool mIsHairBackRInvalidated = false;
    bool mIsSpringControlOff = false;
    PaulineAtCeremonyShadowFunc mShadowMapFunc = nullptr;
};

static_assert(sizeof(PaulineAtCeremony) == 0x190);
