#pragma once

#include <container/seadBuffer.h>
#include <container/seadPtrArray.h>
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

namespace al {
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
static_assert(sizeof(YoshiTongueEatBindInfo) == 0x18);

struct YoshiTongueCollisionBuffer {
    const al::CollisionParts** buffer = nullptr;
    s32 capacity = 0;
    s32 _c = 0;
    s32 size = 0;
};
static_assert(sizeof(YoshiTongueCollisionBuffer) == 0x18);

struct YoshiTongueParam {
    const f32* speed = nullptr;
    const s32* stretchStep = nullptr;
    const f32* range = nullptr;
    const s32* clingWallStep = nullptr;
    const s32* eatStep = nullptr;
    const f32* pullForce = nullptr;
    const f32* pullSpeed = nullptr;
};
static_assert(sizeof(YoshiTongueParam) == 0x38);

class YoshiTongue : public al::LiveActor {
public:
    YoshiTongue(const al::LiveActor* host, const al::LiveActor* modelActor,
                const IUsePlayerCollision* collision,
                const PlayerWallActionHistory* wallActionHistory,
                const PlayerEyeSensorHitHolder* eyeSensorHitHolder,
                const PlayerConst* playerConst, IUsePlayerHack** playerHack, const char* actorName);

    void init(const al::ActorInitInfo& info) override;
    void calcAnim() override;
    void updateCollider() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                    al::HitSensor* self) override;

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
    f32 getTongueParamSpeed() const;
    f32 getTongueParamRange() const;
    bool reactionCollideWall();
    bool reactionCollideGround();
    void returnOrEatHide();
    bool isEnableStayClingGround() const;

    void exeStay();
    void exeStretch();
    void exeHit();
    void exeShrink();
    void exeReturn();
    void exeEat();
    void exeHide();
    void exeClingWall();
    void exeClingGround();

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
    sead::Vector3f _1e0 = sead::Vector3f::zero;
    f32 _1ec = 0.0f;
    f32 _1f0 = 0.0f;
    f32 mShrinkRestRange = 0.0f;
    sead::Vector3f mAttackSensorPos = sead::Vector3f::zero;
    sead::Vector3f mFaceDir = sead::Vector3f::ez;
    bool mIsStayClingGround = false;
    f32 _214 = 0.0f;
};

static_assert(sizeof(YoshiTongue) == 0x218);
