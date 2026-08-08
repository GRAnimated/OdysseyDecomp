#pragma once

#include <basis/seadTypes.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

#include "Player/IUsePlayerPuppet.h"

namespace al {
class AreaObj;
class HitSensor;
class LiveActor;
}  // namespace al

class ActorDimensionKeeper;
class HackCap;
class IPlayerModelChanger;
class IUsePlayerCollision;
class PlayerAnimator;
class PlayerDamageKeeper;
class PlayerConst;
class PlayerCounterForceRun;
class PlayerEffect;
class PlayerInput;
class PlayerJudgePreInputJump;
class WorldEndBorderKeeper;

struct PlayerBindEndJumpInfo {
    sead::Vector3f _0;
    s32 _c;
    f32 gravity;
};
class PlayerPuppet : public IUsePlayerPuppet {
public:
    PlayerPuppet(al::LiveActor* actor, HackCap* hackCap, PlayerAnimator* playerAnimator,
                 IUsePlayerCollision* playerCollision, ActorDimensionKeeper* actorDimensionKeeper,
                 IPlayerModelChanger* playerModelChanger, WorldEndBorderKeeper* worldEndBorderKeeper,
                 PlayerCounterForceRun* playerCounterForceRun, PlayerDamageKeeper* playerDamageKeeper,
                 PlayerEffect* playerEffect, const PlayerInput* playerInput,
                 const PlayerConst* playerConst);

    void start(al::HitSensor* sender, al::HitSensor* receiver) override;
    void end() override;
    void cancel() override;
    void setTrans(const sead::Vector3f& trans) override;
    void setPose(const sead::Quatf& pose) override;
    void setVelocity(const sead::Vector3f& velocity) override;
    void resetPosition(const sead::Vector3f& trans) override;
    const sead::Vector3f& getTrans() const override;
    const sead::Vector3f& getVelocity() const override;
    const sead::Vector3f& getGravity() const override;
    void calcFront(sead::Vector3f* front) const override;
    void calcUp(sead::Vector3f* up) const override;
    void startAction(const sead::SafeString& action) const override;
    bool isActionEnd() const override;
    bool isActionPlaying(const char* action) const override;
    void setAnimRate(f32 rate) const override;
    f32 getAnimFrameMax() const override;
    void startPlayerHitReaction(const char* name);
    void hide() override;
    void show() override;
    bool isHidden() const override;
    void hideSilhouette() override;
    void showSilhouette() override;
    void hideShadow() override;
    void showShadow() override;
    void validateCollisionCheck() override;
    void invalidateCollisionCheck() override;
    bool isValidCollisionCheck() override;
    bool isCollidedGround() override;
    const sead::Vector3f& getCollidedGroundNormal() override;
    bool requestDamage() override;
    void setBindEndJump(const sead::Vector3f& velocity, s32 frames) override;
    void setBindEndWallJump(const sead::Vector3f& velocity, s32 frames);
    void validate2D();
    void keepOn2D();
    void endKeepOn2D();
    void requestUpdateRecoveryInfo(bool isKidsMode, bool isRecovery, const sead::Vector3f& position,
                                   const sead::Vector3f& up, const al::AreaObj* areaObj);
    bool tryUpdateRecoveryInfo(bool* isKidsMode, bool* isRecovery, sead::Vector3f* position,
                               sead::Vector3f* up, const al::AreaObj** areaObj);

    bool isBinding() const;
    bool isNoCollide() const;
    void clearRequestDamage() override;
    bool isRequestDamage() const override;
    void setBindEndOnGround() override;
    bool isBindEndOnGround() const override;
    bool isBindEndJump() const override;
    void validateSensor() override;

    void setJudgePreInputJump(PlayerJudgePreInputJump* judge) { mJudgePreInputJump = judge; }

    bool isEnableGuideArrow() const { return _b4; }

    bool isBindEndCapThrow() const { return mIsBindEndCapThrow; }

    const PlayerBindEndJumpInfo* getBindEndJumpInfo() const { return mBindEndJumpInfo; }

    void invalidateSensor() override;

    bool isDemoPushDisabled() const { return _ac; }

    bool isBindPushDisabled() const { return _ac; }

    bool isBindSeparateCapEnabled() const { return _b3; }

    bool isBindRecoveryEnabled() const { return _b2; }

    bool isSensorValid() const { return mIsSensorValid; }

    bool isWaterSurfaceShadowEnabled() const { return _b5; }

    bool isBindDimensionChangeEnabled() const { return _ae; }

    bool isLookAtEnabled() const { return _b0; }

    bool isLookAtTargetPositionEnabled() const { return _b1; }

    const sead::Vector3f& getLookAtTargetPosition() const { return mLookAtTargetPosition; }

private:
    al::LiveActor* mActor;
    HackCap* mHackCap;
    PlayerAnimator* mPlayerAnimator;
    IUsePlayerCollision* mIUsePlayerCollision;
    ActorDimensionKeeper* mActorDimensionKeeper;
    IPlayerModelChanger* mIPlayerModelChanger;
    WorldEndBorderKeeper* mWorldEndBorderKeeper;
    PlayerCounterForceRun* mPlayerCounterForceRun;
    PlayerDamageKeeper* mPlayerDamageKeeper;
    PlayerEffect* mPlayerEffect;
    PlayerJudgePreInputJump* mJudgePreInputJump;
    const PlayerInput* mPlayerInput;
    const PlayerConst* mPlayerConst;
    al::HitSensor* _70;
    al::HitSensor* _78;
    bool _80;
    bool _81;
    bool _82;
    sead::Vector3f _84;
    sead::Vector3f _90;
    const al::AreaObj* mAreaObj;
    bool mIsBindEndOnGround;
    bool mIsBindEndJump;
    bool mIsBindEndCapThrow;
    bool mIsValidCollisionCheck;
    bool _ac;
    bool mIsRequestDamage;
    u8 _ae;
    bool mIsSensorValid;
    bool _b0;
    bool _b1;
    bool _b2;
    bool _b3;
    bool _b4;
    bool _b5;
    bool _b6;
    u8 _b7;
    sead::Vector3f mLookAtTargetPosition;
    u8 mPaddingC4[4];
    PlayerBindEndJumpInfo* mBindEndJumpInfo;
};

static_assert(sizeof(PlayerPuppet) == 0xd0);
