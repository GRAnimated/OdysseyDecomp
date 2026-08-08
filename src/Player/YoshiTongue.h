#pragma once

#include <container/seadBuffer.h>
#include <container/seadPtrArray.h>
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

namespace al {
struct ActorParamF32;
struct ActorParamS32;
class CollisionParts;
class HitSensor;
class SensorMsg;
}  // namespace al

class IJudge;
class IUsePlayerCollision;
class IUsePlayerHack;
class PlayerConst;
class PlayerEyeSensorHitHolder;
class PlayerWallActionHistory;
class YoshiTongueCollider;
class YoshiJudgeStartTongueClingFix;
class YoshiTongueJointControlKeeper;
class YoshiTongueTipConnector;

struct YoshiTongueEatBindInfo {
    al::HitSensor* sensor = nullptr;
    f32 scale = 1.0f;
    f32 _c = 0.0f;
    s32 _10 = 0;
};
struct YoshiTongueCollisionBuffer {
    const al::CollisionParts** buffer = nullptr;
    s32 capacity = 0;
    s32 _c = 0;
    s32 size = 0;
};
struct YoshiTongueParam {
    al::ActorParamF32* speed;
    al::ActorParamS32* stretchStep;
    al::ActorParamF32* range;
    al::ActorParamS32* clingWallStep;
    al::ActorParamS32* eatStep;
    al::ActorParamF32* pullForce;
    al::ActorParamF32* pullSpeed;
};
class YoshiTongue : public al::LiveActor {
public:
    YoshiTongue(const al::LiveActor* host, const al::LiveActor* modelActor,
                const IUsePlayerCollision* collision,
                const PlayerWallActionHistory* wallActionHistory,
                const PlayerEyeSensorHitHolder* eyeSensorHitHolder,
                const PlayerConst* playerConst, IUsePlayerHack** playerHack, const char* actorName);

    void init(const al::ActorInitInfo& info) override;
    void updateCollider() override;
    void updateEatBindActor();
    void calcAnim() override;
    void startAttack(const sead::Vector3f& startPos, const sead::Vector3f& direction);
    void startShrink();

    void endShrink();
    void eatFinish();
    void endHack();
    bool isEnableStartAttack() const;
    bool isEnableLookAtTip() const;
    bool isEnableShrinkStart() const;
    bool isEnableEatFinish() const;
    bool isExistEatBind() const;
    bool isShrinkMove() const;
    bool isConnectWall() const;
    bool isConnectGround() const;
    void calcYoshiFaceDir(sead::Vector3f* faceDir) const;
    void calcTongueTipPos(sead::Vector3f* tipPos) const;
    bool tryCalcTonguePullForce(f32* force, sead::Vector3f* direction) const;
    bool tryCalcTonguePullDistance(sead::Vector3f* distance) const;
    bool tryCalcTongueConnect(const al::CollisionParts** collisionParts,
                              sead::Vector3f* connectPos, sead::Vector3f* connectNormal,
                              sead::Vector3f* direction, sead::Vector3f* tipPos) const;
    f32 getShrinkRestRange() const;
    void adjustShrinkRestRange(f32 range);
    void exeStretch();
    f32 getTongueParamSpeed() const;
    f32 getTongueParamRange() const;
    bool reactionCollideWall();
    bool reactionCollideGround();
    void returnOrEatHide();
    void exeStay();
    void exeHit();

    void exeClingWall();
    void exeClingGround();
    void exeShrink();
    void exeReturn();
    void exeEat();
    void exeHide();
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                    al::HitSensor* self) override;
    bool isEnableStayClingGround() const;

private:
    const al::LiveActor* mHost;
    const al::LiveActor* mModelActor;
    const IUsePlayerCollision* mCollision;
    const PlayerWallActionHistory* mWallActionHistory;
    const PlayerEyeSensorHitHolder* mEyeSensorHitHolder;
    const PlayerConst* mPlayerConst;
    IUsePlayerHack** mPlayerHack;
    YoshiTongueCollider* mTongueCollider = nullptr;
    YoshiTongueJointControlKeeper* mJointControlKeeper = nullptr;
    YoshiTongueTipConnector* mTipConnector = nullptr;
    YoshiJudgeStartTongueClingFix* mJudgeStartClingFix = nullptr;
    YoshiTongueParam* mParam = nullptr;
    YoshiTongueCollisionBuffer mCollisionBuffer;
    sead::PtrArray<YoshiTongueEatBindInfo> mEatBindInfo;
    sead::Buffer<YoshiTongueEatBindInfo*> mEatBindInfoBuffer;
    bool mIsHack = false;
    sead::Vector3f mStartPos = sead::Vector3f::zero;
    sead::Vector3f mTongueDir = sead::Vector3f::zero;
    sead::Vector3f mUpDir = sead::Vector3f::zero;
    sead::Vector3f mTongueTipPos = sead::Vector3f::zero;
    sead::Vector3f mVelocity = sead::Vector3f::zero;
    s32 _1e0 = 0;
    f32 _1e4 = 0.0f;
    sead::Vector3f mReturnOffset = sead::Vector3f::zero;
    f32 mShrinkRestRange = 0.0f;
    sead::Vector3f mAttackSensorPos = sead::Vector3f::zero;
    sead::Vector3f mFaceDir = sead::Vector3f::ez;
    bool mIsStayClingGround = false;
    f32 _214 = 0.0f;
};

static_assert(sizeof(YoshiTongue) == 0x218);
