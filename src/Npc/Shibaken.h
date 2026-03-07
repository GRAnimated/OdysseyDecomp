#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

struct ActorInitInfo;
class ActorStateReactionBase;
class NpcJointLookAtController;
class PlayerPushReceiver;
class ShibakenMoveAnimCtrl;
class ShibakenStateBark;
class ShibakenStateCapCatch;
class ShibakenStateJump;
class ShibakenStatePointChase;
class ShibakenStateSit;
class ShibakenStateWait;
class ShibakenStateWaitFar;
class TalkNpcCap;

namespace al {
class JointControllerBase;
}

class Shibaken : public al::LiveActor {
public:
    Shibaken(const char* name);

    void init(const al::ActorInitInfo& info) override;

    void exeSit();
    void exeWaitInit();
    void exeWait();
    void exeWaitFar();
    void exeFindTurn();
    void exeFind();
    void exePlayerChase();
    void exePlayerChaseTurn();
    void exePointChase();
    void exeCapCatch();
    void exeBark();
    void exeSleepStart();
    void exeSleep();
    void exeSleepEnd();
    void exeJump();
    void exeReaction();
    void exeReset();

    void* mCapTarget = nullptr;
    TalkNpcCap* mTalkNpcCap = nullptr;
    ShibakenMoveAnimCtrl* mMoveAnimCtrl = nullptr;
    NpcJointLookAtController* mJointLookAt = nullptr;
    al::JointControllerBase* mChestJoint = nullptr;
    void* mDigPointHolder = nullptr;
    ShibakenStateSit* mStateSit = nullptr;
    ShibakenStateWait* mStateWait = nullptr;
    ShibakenStateWaitFar* mStateWaitFar = nullptr;
    ShibakenStatePointChase* mStatePointChase = nullptr;
    ShibakenStateCapCatch* mStateCapCatch = nullptr;
    ShibakenStateBark* mStateBark = nullptr;
    ShibakenStateJump* mStateJump = nullptr;
    ActorStateReactionBase* mStateReaction = nullptr;
    PlayerPushReceiver* mPushReceiver = nullptr;
    s32 mWorldMoonFlag = 0;
    sead::Vector3f mInitTrans = sead::Vector3f::zero;
    sead::Quatf mInitQuat = sead::Quatf::unit;
    sead::Quatf _1a0 = sead::Quatf::unit;
    sead::Quatf _1b0 = sead::Quatf::unit;
    f32 mRangeSensorRadius = 3000.0f;
    void* _1c8 = nullptr;
    bool mIsHomeShip = false;
    bool mIsInvalidJumpLowWall = false;
};

static_assert(sizeof(Shibaken) == 0x1D8);
