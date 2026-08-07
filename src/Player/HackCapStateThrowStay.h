#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class HitSensor;
class LiveActor;
}  // namespace al
class HackCapJudgePreInputSeparateJump;
class HackCapJointControlKeeper;
class HackCapTrigger;
class IUsePlayerCollision;
class PlayerColliderHackCap;
class PlayerEyeSensorHitHolder;
class PlayerInput;
class PlayerSeparateCapFlag;

class HackCapStateThrowStay : public al::ActorStateBase {
public:
    HackCapStateThrowStay(al::LiveActor*, const PlayerColliderHackCap*, const al::LiveActor*,
                          const PlayerSeparateCapFlag*, const PlayerInput*,
                          const IUsePlayerCollision*, const PlayerEyeSensorHitHolder*,
                          const HackCapTrigger*, HackCapJointControlKeeper*,
                          HackCapJudgePreInputSeparateJump*, const bool*);
    ~HackCapStateThrowStay() override;

    void appear() override;
    void kill() override;
    bool update() override;
    bool isHomingPlayerJump() const;
    bool isEnableAppendAttack() const;
    bool isEnableKeepStayTouchJump() const;
    bool isEnableTouchJumpTransWarp() const;
    bool isEnableSendHipDropMsg() const;
    bool sendHipDropCollideMsg(al::HitSensor*);
    bool sendHipDropObjMsg(HackCapTrigger*, al::HitSensor*, al::HitSensor*);
    void exeStay();
    void exeSeparateMove();
    void updateStayMove();
    void exeSeparateJump();
    void exeSeparateHomingAttack();
    void exeSeparateHipDropStart();
    void exeSeparateHipDropLoop();
    void exeSeparateHipDropLand();
    void exeSeparateFallDown();
    void exeSeparateApproachStart();
    void exeSeparateApproach();
    void exeSeparateApproachEnd();

private:
    const PlayerColliderHackCap* mCollider;
    const al::LiveActor* mPlayer;
    const PlayerSeparateCapFlag* mSeparateCapFlag;
    const PlayerInput* mInput;
    const IUsePlayerCollision* mCollision;
    const PlayerEyeSensorHitHolder* mEyeSensorHitHolder;
    const HackCapTrigger* mTrigger;
    HackCapJointControlKeeper* mJointControlKeeper;
    HackCapJudgePreInputSeparateJump* mJudgePreInputSeparateJump;
    const u8* _68;
    bool _70;
    bool _71;
    bool _72;
    u8 _73;
    sead::Vector3f _74;
    sead::Vector3f _80;
    sead::Vector3f _8c;
    f32 _98;
    bool _9c;
    u8 _9d[3];
    sead::Vector3f _a0;
    u8 _ac[4];
    const char* _b0;
    f32 _b8;
    f32 _bc;
    bool _c0;
    u8 _c1[3];
    s32 _c4;
    bool _c8;
    u8 _c9[3];
    s32 _cc;
    s32 _d0;
    sead::Vector3f _d4;
    al::HitSensor* _e0;
    sead::Vector3f _e8;
    s32 _f4;
    f32 _f8;
    sead::Vector3f _fc;
    s32 _108;
    bool _10c;
    u8 _10d[3];
};

static_assert(sizeof(HackCapStateThrowStay) == 0x110);
