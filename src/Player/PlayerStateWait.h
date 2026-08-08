#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class AreaObj;
class HitSensor;
class LiveActor;
class MtxConnector;
class WaterSurfaceFinder;
}  // namespace al
class IJudge;
class IUsePlayerCollision;
class IUsePlayerHeightCheck;
class PlayerAnimator;
class PlayerCapManHeroEyesControl;
class PlayerConst;
class PlayerJointControlKeeper;
struct PlayerJointParamCenterDynamics;
struct PlayerJointParamGroundPose;
class PlayerModelChangerHakoniwa;
class PlayerTrigger;

class PlayerStateWait : public al::ActorStateBase {
public:
    PlayerStateWait(al::LiveActor* player, const PlayerConst* pConst,
                    const IUsePlayerCollision* collision,
                    const PlayerModelChangerHakoniwa* modelChanger,
                    const PlayerJointControlKeeper* jointControlKeeper,
                    const al::WaterSurfaceFinder* waterSurfaceFinder,
                    const IUsePlayerHeightCheck* heightCheck, const IJudge* judgeInWater,
                    PlayerAnimator* animator, PlayerTrigger* trigger,
                    PlayerCapManHeroEyesControl* capManHeroEyesControl,
                    PlayerJointParamCenterDynamics* centerDynamics,
                    PlayerJointParamGroundPose* groundPose);
    void appear() override;
    void kill() override;
    void control() override;
    bool isWait() const;
    bool isEnableLookAt() const;
    bool isEnableCancelAction() const;
    bool isEnableCancelHipDropJump() const;
    bool isEnableRecoveryLife() const;
    bool isEnableReactionCapCatch() const;
    bool isAreaAnimCapManHeroEyes() const;
    bool isPlaySwitchOnAreaAnim() const;
    bool isLandStain() const;
    bool isSleep() const;
    bool attackHipDropKnockDown(al::HitSensor* self, al::HitSensor* other);
    bool tryClearIgnoreSwitchOnAreaAnim();
    bool requestAnimName(const char* animationName);
    void noticeCarryStart();
    void initSceneStartAnim();
    bool tryConnectWait();
    f32 getInverseKinematicsRate() const;
    void exeLandStiffen();
    void exeLand();
    void exeWait();
    bool tryUpdateAreaAnim();
    bool tryChangeRequestAnim();
    bool tryChangeAreaAnim();
    bool tryGetSpecialStatusAnimName(const char** animationName);
    void exeWaitConnect2D();
    void exeWaitRelaxStart();
    void exeWaitRelax();
    void exeWaitSnoozeStart();
    void exeWaitSnooze();
    void exeWaitSleepStart();
    void exeWaitSleep();
    void exeAreaAnimTurn();
    void exeAreaAnimWait();
    void exeRequestAnimWait();
~PlayerStateWait() override;

private:
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollision;
    const PlayerModelChangerHakoniwa* mModelChanger;
    const PlayerJointControlKeeper* mJointControlKeeper;
    const al::WaterSurfaceFinder* mWaterSurfaceFinder;
    const IUsePlayerHeightCheck* mHeightCheck;
    const IJudge* mJudgeInWater;
    PlayerAnimator* mAnimator;
    PlayerTrigger* mTrigger;
    PlayerCapManHeroEyesControl* mCapManHeroEyesControl;
    PlayerJointParamCenterDynamics* mCenterDynamics;
    PlayerJointParamGroundPose* mGroundPose;
    bool _80 = false;
    u8 _81[7];
    al::AreaObj* mAreaAnimArea = nullptr;
    const char* mAreaAnimName = nullptr;
    sead::Vector3f mAreaAnimSnapFront{};
    s32 mAreaAnimStartFrame = 0;
    s32 _a8 = 0;
    bool mIsUseAreaValid = false;
    bool mIsAreaWaitSitDown = false;
    bool mIsIgnoreLifeOne = false;
    u8 _af = 0;
    s32 _b0 = 0;
    u8 mIsInvalidateInput = 0;
    u8 _b5[3];
    const char* mRequestAnimName = nullptr;
    sead::Vector3f _c0{};
    s32 _cc = 0;
    al::MtxConnector* mMtxConnector;
    sead::Vector3f mConnectTrans{};
    bool _e4 = false;
    u8 _e5[3];
};

static_assert(sizeof(PlayerStateWait) == 0xE8);
