#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveExecutor.h"

#include "Player/PlayerJointParamHandLegAngle.h"

namespace al {
class LiveActor;
class HitSensor;
}  // namespace al
class IUsePlayerCollision;
class PlayerAnimator;
class IPlayerModelChanger;
class IUsePlayerCeilingCheck;
class PlayerPushReceiver;

class PlayerCarryKeeper : public al::NerveExecutor {
public:
    PlayerCarryKeeper(const al::LiveActor* player, al::HitSensor* carrySensor,
                      PlayerAnimator* animator, const IPlayerModelChanger* modelChanger,
                      const IUsePlayerCeilingCheck* ceilingCheck,
                      PlayerJointParamHandLegAngle* handLegAngleParam);

    void update();
    bool updateCollideLockUp(const IUsePlayerCollision* collider,
                             const PlayerPushReceiver* pushReceiver);
    void updateHandJointAngle();

    bool startCarry(al::HitSensor* heldSensor);
    bool startThrow(bool swing);
    void startCancelAndRelease();
    void startRelease();
    void startReleaseDemo();
    void startReleaseDamage();
    void startReleaseDead();
    void startDemoKeepCarry();
    void startDemoShineGet();
    void endDemoShineGet();
    void startCameraSubjective();
    void endCameraSubjective();
    void sendPlayerWarp();

    bool isCarry() const;
    bool isCarryWallKeep() const;
    bool isCarryFront() const;
    bool isCarryUp() const;
    bool isThrow() const;
    bool isThrowHandR() const;
    bool isThrowHold() const;
    bool isThrowRelease() const;
    s32 getThrowReleaseFrame() const;

    const char* getCarryStartAnimName() const;
    const char* getCarryThrowAnimName() const;
    const char* getCarryAnimName() const;

    void exeWait();
    void exeStart();
    void exeCarry();
    void exeThrow();
    void exeRelease();

private:
    al::LiveActor* mPlayer;
    PlayerAnimator* mAnimator;
    al::HitSensor* mCarrySensor;
    al::HitSensor* mHeldSensor;
    IPlayerModelChanger* mModelChanger;
    IUsePlayerCeilingCheck* mCeilingCheck;
    PlayerJointParamHandLegAngle* mHandLegAngleParam;
    s32 mCarryDelay;

    union {
        struct {
            bool carryAboveFlag;
            bool carryWallKeepFlag;
        };

        u16 mCarryFlags;
    };

    sead::Vector3f mHandJointAngle;
    s32 mCollideLockCounter;
    sead::Vector3f mCarryActorPos;
};

static_assert(sizeof(PlayerCarryKeeper) == 0x70);
