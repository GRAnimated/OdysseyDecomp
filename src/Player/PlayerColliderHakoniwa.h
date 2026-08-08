#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Player/IUsePlayerCeilingCheck.h"
#include "Player/IUsePlayerCollision.h"
#include "Player/IUsePlayerFallDistanceCheck.h"
#include "Player/IUsePlayerHeightCheck.h"

namespace al {
class CollisionPartsFilterBase;
class HitSensor;
class LiveActor;
}  // namespace al

class CollisionShapeKeeper;
class PlayerCeilingCheck;
class PlayerConst;

class PlayerColliderHakoniwa : public IUsePlayerCollision,
                               public IUsePlayerCeilingCheck,
                               public IUsePlayerHeightCheck,
                               public IUsePlayerFallDistanceCheck {
public:
    PlayerColliderHakoniwa(al::LiveActor* player, const PlayerConst* playerConst);

    void init();
    sead::Vector3f updateCollider(const sead::Vector3f& velocity);
    void updateHeightCheck(const sead::Vector3f& trans, const sead::Vector3f& up, bool isGrounded);
    void updateCeilingCheck(const sead::Vector3f& trans, const sead::Vector3f& up,
                            f32 headClearance, f32 holdHeight);
    void updateFallDistanceCheck(const sead::Vector3f& trans, const sead::Vector3f& velocity,
                                 const sead::Vector3f& gravity, f32 threshold);

    void changeCollisionNormal();
    void changeCollisionMini();
    void changeCollisionSwim();
    void changeCollisionWallGrab(const sead::Vector3f& cutDir);
    void changeCollisionGrabCeil(const sead::Vector3f& cutDir);
    void changeCollisionPoleClimb(const sead::Vector3f& cutDir);
    void changeCollision2DNormal();
    void changeCollision2DMini();
    void changeCollisionHack(const al::CollisionPartsFilterBase* filter);
    void changeCollisionRecovery(bool is2D);
    void changeCollisionAbyss(const sead::Vector3f& cutDir);
    void prepareCollisionEndHack();
    void prepareCollisionEndRecovery();

    f32 getColliderRadius() const;
    f32 getColliderDiskHalfHeight() const;
    void calcBoundingCenter(sead::Vector3f* center) const;
    void calcSeparateCapLocalOffset(sead::Vector3f* offset) const;
    void validateGroundSupport();
    void invalidateGroundSupport();
    bool isEnableStandUp() const override;
    bool isEnableHoldUp() const override;
    bool isPressedCeil() const override;
    f32 getSafetyCeilSpace() const override;
    f32 getCeilCheckHeight() const override;
    PlayerCollider* getPlayerCollider() const override;
    bool isAboveGround() const override;
    f32 getGroundHeight() const override;
    f32 getShadowDropHeight() const override;
    f32 getFallDistance() const override;
    void resetFallDistance() {
        mFallStartPos.set(0.0f, 0.0f, 0.0f);
        mFallDistance = 0.0f;
    }

    al::HitSensor* getGroundSensor() const { return mGroundSensor; }
    s32 getNoGroundFrames() const { return mNoGroundFrames; }
    const sead::Vector3f& getGroundNormal() const { return mGroundNormal; }
    al::CollisionPartsFilterBase* getCollisionFilter2D() const { return mCollisionFilter2D; }

private:
    al::LiveActor* mPlayer;
    const PlayerConst* mConst;
    PlayerCollider* mCollider = nullptr;
    PlayerCeilingCheck* mCeilingCheck = nullptr;
    bool mIsAboveGround = false;
    u8 _41[3];
    f32 mGroundHeight = 0.0f;
    f32 mShadowDropHeight = 0.0f;
    sead::Vector3f mGroundNormal = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mFallStartPos = {0.0f, 0.0f, 0.0f};
    f32 mFallDistance = 0.0f;
    s32 mNoGroundFrames = 0;
    u32 _6c;
    al::HitSensor* mGroundSensor = nullptr;
    CollisionShapeKeeper* mCollisionNormal = nullptr;
    CollisionShapeKeeper* mCollisionMini = nullptr;
    CollisionShapeKeeper* mCollisionSwim = nullptr;
    CollisionShapeKeeper* mCollisionGrabCeil = nullptr;
    CollisionShapeKeeper* mCollisionWallGrab = nullptr;
    CollisionShapeKeeper* mCollisionPoleClimb = nullptr;
    CollisionShapeKeeper* mCollision2DNormal = nullptr;
    CollisionShapeKeeper* mCollision2DMini = nullptr;
    al::CollisionPartsFilterBase* mCollisionFilter2D;
};

static_assert(sizeof(PlayerColliderHakoniwa) == 0xc0);
