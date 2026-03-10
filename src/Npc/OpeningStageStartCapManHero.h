#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/LiveActor/LiveActor.h"

#include "Demo/IUseDemoSkip.h"

namespace al {
class AddDemoInfo;
class CameraTicket;
class DemoActorHolder;
class EventFlowExecutor;
class JointSpringControllerHolder;
class RailKeeper;
class RailRider;
struct ActorInitInfo;
}  // namespace al

class EventActorStateDemoTalk;
class QuestInfo;
class SimpleActionPartsModel;

class OpeningStageStartCapManHero : public al::LiveActor,
                                    public al::IEventFlowEventReceiver,
                                    public IUseDemoSkip {
public:
    OpeningStageStartCapManHero(const char* name);

    void init(const al::ActorInitInfo& initInfo) override;
    void control() override;
    al::RailRider* getRailRider() const override;

    bool receiveEvent(const al::EventFlowEventData* eventData) override;

    bool isFirstDemo() const override;
    bool isEnableSkipDemo() const override;
    void skipDemo() override;
    void updateOnlyDemoGraphics() override;

    void startDemo();
    void endDemo(bool isSkip);
    void restartAfterOpening();
    bool isStartRunAway() const;
    void noticePlayerStand();
    void startRunAway();
    void startWait();

    void exeWander();
    void exeWanderDoubtTouch();
    void exeNoticePlayerStand();
    bool runAwayToRail();
    void exeWaitPlayerTurn();
    void exeSurprise();
    void exeRunAway();
    void exeRunAwayWait();
    void exeWaitBridge();
    void exeEventTalkDemo();
    void exeEventKoopaDemo();

private:
    bool mIsStartDemo = true;
    bool _119 = false;
    void* mPlayerRestartPos = nullptr;
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    EventActorStateDemoTalk* mDemoTalkState = nullptr;
    sead::Matrix34f mKoopaDemoMtx = sead::Matrix34f::ident;
    al::DemoActorHolder* mDemoActorHolder = nullptr;
    void* mTalkStartPos = nullptr;
    al::CameraTicket* mEntranceCamera = nullptr;
    al::CameraTicket* mDemoAnimCamera = nullptr;
    void* _188 = nullptr;
    void* _190 = nullptr;
    void* _198 = nullptr;
    f32 mRunSpeed = 0.0f;
    al::RailKeeper* mWaitRailKeeper = nullptr;
    s32 mWaitTimer = 0;
    sead::Vector3f mBridgeStartPos;
    sead::Vector3f mBridgeEndPos;
    al::LiveActor* mMarioCap = nullptr;
    al::LiveActor* mMarioCapEye = nullptr;
    SimpleActionPartsModel* mCapBroken = nullptr;
    al::JointSpringControllerHolder* mJointSpringControllerHolder = nullptr;
    bool mIsClearSklAnim = false;
    bool mIsScreenCoverActive = false;
    al::AddDemoInfo* mAddDemoInfo = nullptr;
    QuestInfo* mQuestInfo = nullptr;
    f32 mPrevRailCoord = 0.0f;
};

static_assert(sizeof(OpeningStageStartCapManHero) == 0x210);
