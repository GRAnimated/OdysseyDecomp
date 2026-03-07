#pragma once

#include <math/seadVector.h>
#include <prim/seadSafeString.h>

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/LiveActor/LiveActor.h"

namespace al {
class EventFlowExecutor;
class Nerve;
struct ActorInitInfo;
}  // namespace al

class NpcJointLookAtController;
class NpcStateTurnSeparate;
class TalkNpcCap;

using IsPlayerHackFn = bool (*)(const al::LiveActor*);
using CalcJointDirFn = void (*)(sead::Vector3f*, const al::LiveActor*, const char*);

class RunAwayNpc : public al::LiveActor, public al::IEventFlowEventReceiver {
public:
    RunAwayNpc(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void initAfterPlacement() override;
    void control() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                    al::HitSensor* self) override;
    bool receiveEvent(const al::EventFlowEventData* event) override;

    void exePatrol();
    const char* getCurrentWaitActionName();
    void reactToPlayer();
    void exeReaction();
    void exeSurprised();
    void exeScaredToRun();
    void exeScaredByRegularEnemy();
    void exeRelieved();
    void resetScareUneffectiveWindow();
    void exePrepareToRunAway();
    void exeRunAway();
    void exeHiding();
    void exeHidingAndTurning();
    void exeTurn();
    void exeTurnToPlacementFront();
    bool canSeeWithCylindricalLineOfSight(const sead::Vector3f& pos);

private:
    NpcStateTurnSeparate* mTurnState = nullptr;
    TalkNpcCap* mCap = nullptr;
    NpcJointLookAtController* mLookAtCtrl = nullptr;
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    s32 mHackType = 0;
    s32 _134;
    sead::FixedSafeString<32> mSurpriseActionName;
    sead::FixedSafeString<32> mSearchActionName;
    u8 mReactionType = 0;
    u8 _1a9;
    u8 _1aa;
    u8 _1ab;
    f32 mNormalizedDistToPlayer = __FLT_MAX__;
    const al::Nerve* mStoredNerve = nullptr;
    f32 mRunDirection = 1.0f;
    s32 _1bc;
    IsPlayerHackFn mIsPlayerHack = nullptr;
    CalcJointDirFn mCalcJointDir = nullptr;
    sead::Vector3f mPlacementFront;
    s32 mWaitActionTimer = 40;
    s16 mScareType = 0;
    bool mIsHipJoint = false;
    u8 _1e3;
    sead::Vector3f mEnemySensorPos;
    s32 mScareUneffectiveWindow = 0;
    s32 mDefaultAction = 0;
    bool mIsTurnedToPlayer = true;
    bool mIsRunning = false;
    u8 _1fa;
    u8 _1fb;
    s32 mReactionMaxStep = 0;
    bool mIsPlayerClose = false;
    bool _201 = true;
};

static_assert(sizeof(RunAwayNpc) == 0x208);
