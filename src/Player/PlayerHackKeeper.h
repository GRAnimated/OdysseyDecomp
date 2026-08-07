#pragma once

#include <container/seadPtrArray.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Player/PlayerHackStartTexKeeper.h"

namespace al {
class LiveActor;
class HitSensor;
class CollisionPartsFilterBase;
struct ActorInitInfo;
}  // namespace al

class PlayerRecoverySafetyPoint;
class HackCap;
class PlayerInput;
class PlayerJudgePreInputJump;
class PlayerJudgePreInputHackAction;
class PlayerDamageKeeper;
class IPlayerModelChanger;
class IUsePlayerHeightCheck;
class IUsePlayerCollision;
struct HackObjInfo;
class PlayerCollider;
class CapTargetInfo;
class IUsePlayerHack;

struct HackEndParam {
    sead::Vector3f vel = {0.0f, 0.0f, 0.0f};
    sead::Vector3f dir = {0.0f, 0.0f, 0.0f};
    sead::Quatf quat = sead::Quatf::unit;
    sead::Vector3f targetPos = {0.0f, 0.0f, 0.0f};
    f32 escapeScale = 1.0f;
    s32 delayFrames = 0;
    bool isDamage = false;
    bool hasVelocity = false;
};

static_assert(sizeof(HackEndParam) == 0x40);

class PlayerHackKeeper {
public:
    friend class PlayerActorHakoniwa;

    PlayerHackKeeper(al::LiveActor* player, HackCap* cap, PlayerRecoverySafetyPoint* safetyPoint,
                     const PlayerInput* input, const sead::Matrix34f* mtx,
                     const PlayerDamageKeeper* damageKeeper,
                     const IPlayerModelChanger* modelChanger,
                     const IUsePlayerHeightCheck* heightCheck);

    void createHackModel(const al::ActorInitInfo&);
    void startHack(al::HitSensor*, al::HitSensor*, al::LiveActor*);
    void setupHack(al::HitSensor*, al::HitSensor*, al::LiveActor*);
    void endHack(const HackEndParam*);
    void endHackStartDemo(al::LiveActor*);
    void startHackStartDemo(al::LiveActor*);
    void startHackStartDemoPuppetable(al::LiveActor*);
    void addHackStartDemo(al::LiveActor*);
    void appearHackDemoModel(const sead::Matrix34f&, f32);
    void updateHackDemoModel(const sead::Matrix34f&, f32);
    void deleteHackDemoModelEffect();
    void killHackDemoModel();
    bool isActiveHackStartDemo() const;
    void recordHack();
    bool cancelHackArea();
    void cancelHack();
    void cancelForceRecovery();
    bool tryEscapeHack();
    bool sendTransferHack();
    void sendMarioDemo();
    void sendMarioCheckpointFlagWarp();
    void forceKillHack();
    void sendMarioDead();
    bool sendMarioInWater();
    bool sendMarioDeathArea();
    void sendMsgEnableMapCheckPointWarp();
    bool sendMsgSelfCeilingCheckMiss();
    bool receiveRequestTransferHack(al::HitSensor*);
    bool requestDamage();
    bool receiveRequestDamage();
    void sendSyncDamageVisibility();
    void pushWorldEndBorder(const sead::Vector3f&);
    const char* getCurrentHackName() const;
    IUsePlayerCollision* getPlayerCollision() const;
    f32 getHackGuideHeight() const;
    bool isHackGuideEnable() const;
    f32 getHackStayGravityMargine() const;
    al::CollisionPartsFilterBase* getCollisionPartsFilter() const;
    bool isHackGroupTalkScare() const;
    bool isHackNoCollisionMsg() const;
    bool isHackNoSeparateCameraInput() const;
    bool isHackUsePlayerCollision() const;
    bool isHackCancelCeilingCheck() const;
    bool isHackInvalidLifeRecovery() const;
    void requestForceHackStageStart(al::HitSensor*, const CapTargetInfo*, al::LiveActor*);
    bool executeForceHackStageStart(al::HitSensor*, IUsePlayerHack*);
    void startDemo();
    void endDemo();

    al::LiveActor* getParent() const { return mParent; }

    HackCap* getHackCap() const { return mHackCap; }

    PlayerRecoverySafetyPoint* getRecoverySafePoint() const { return mRecoverySafePoint; }

    PlayerInput* getInput() const { return mInput; }

    PlayerDamageKeeper* getDamageKeeper() const { return mDamageKeeper; }

    IPlayerModelChanger* getModelChanger() const { return mModelChanger; }

    IUsePlayerHeightCheck* getHeightCheck() const { return mHeightCheck; }

    void setPuppetable(bool isPuppetable) { mIsPuppetable = isPuppetable; }

    void setPreInputJudges(PlayerJudgePreInputJump* jump, PlayerJudgePreInputHackAction* hack) {
        mJudgePreInputJump = jump;
        mJudgePreInputHackAction = hack;
    }

    bool isPuppetable() const { return mIsPuppetable; }

    bool isCancellingHack() const { return mIsCancellingHack; }

    bool isPuppetable2() const { return mIsPuppetable2; }

    bool isHack() const { return mIsHack; }

    bool isHackDemoStarted() const { return mIsHackDemoStarted; }

    bool isStartedHacking() const { return mIsStartedHacking; }

    void setStartedHacking(bool isStartedHacking) { mIsStartedHacking = isStartedHacking; }

    void setHack(bool isHack) { mIsHack = isHack; }

    al::LiveActor* getHack() const { return mHackActor; }

    al::HitSensor* getHackSensor() const { return mHackHitSensor; }

private:
    al::LiveActor* mParent;
    HackCap* mHackCap;
    PlayerRecoverySafetyPoint* mRecoverySafePoint;
    PlayerJudgePreInputJump* mJudgePreInputJump;
    PlayerJudgePreInputHackAction* mJudgePreInputHackAction;
    PlayerInput* mInput;
    sead::Matrix34f* field_30;
    PlayerDamageKeeper* mDamageKeeper;
    IPlayerModelChanger* mModelChanger;
    IUsePlayerHeightCheck* mHeightCheck;
    al::HitSensor* mParentBodySensor;
    bool mIsPuppetable;
    bool mIsCancellingHack;
    bool mIsHackDemoStarted;
    bool mIsPuppetable2;
    bool mIsStartedHacking;
    bool mIsHack;
    bool mIsTookDamage;
    al::CollisionPartsFilterBase* mCollisionFilter;
    al::LiveActor* mHackActor;
    al::HitSensor* mHackHitSensor;
    HackObjInfo* mHackObjectInfo;
    al::HitSensor* mStageStartActorSensor;
    al::LiveActor* mStageStartActor;
    CapTargetInfo* mStageStartCapTargetInfo;
    PlayerHackStartTexKeeper* mHackStartTexKeeper;
    al::LiveActor* mHackModel;
    sead::PtrArray<sead::Matrix34f> mHackModelSlices;
    sead::Matrix34f* field_b8;
    s32 field_c0;
    s32 field_c4;
    s32 field_c8;
};

static_assert(sizeof(PlayerHackKeeper) == 0xd0);
