#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Player/IUsePlayerCeilingCheck.h"
#include "Player/IUsePlayerCollision.h"
#include "Player/IUsePlayerFallDistanceCheck.h"
#include "Player/IUsePlayerHeightCheck.h"

namespace al {
class LiveActor;
}

class PlayerCollider;
class PlayerColliderDisk;
class PlayerCollisionCheckSphereMove;
class PlayerConst;

class PlayerCollider2D3D : public IUsePlayerCollision,
                           public IUsePlayerCeilingCheck,
                           public IUsePlayerHeightCheck,
                           public IUsePlayerFallDistanceCheck {
public:
    PlayerCollider2D3D(al::LiveActor* player, const PlayerConst* playerConst,
                       PlayerCollider* collider, PlayerColliderDisk* colliderDisk);

    void setFlag2D(bool is2D);
    sead::Vector3f updateCollider(const sead::Vector3f& velocity);
    void updateHeightCheck(const sead::Vector3f& trans, const sead::Vector3f& up);
    void updateCeilingCheck(const sead::Vector3f& trans, const sead::Vector3f& up);
    void updateFallDistanceCheck(const sead::Vector3f& trans, const sead::Vector3f& velocity,
                                 const sead::Vector3f& gravity, f32 threshold);
    void changeCollisionMini();
    void changeCollisionNormal();
    void changeCollisionWallGrab();
    f32 getColliderRadius() const;
    f32 getColliderDiskHalfHeight() const;
    void calcDiskColliderBoundingCenter(sead::Vector3f* center) const;

    PlayerCollider* getPlayerCollider() const override;
    bool isEnableStandUp() const override;
    bool isEnableHoldUp() const override;
    bool isPressedCeil() const override;
    bool isAboveGround() const override;
    f32 getGroundHeight() const override;
    f32 getFallDistance() const override;

private:
    al::LiveActor* mPlayer;
    const PlayerConst* mConst;
    bool mIs2D;
    u8 _31[7];
    PlayerCollider* mCollider;
    PlayerColliderDisk* mColliderDisk;
    PlayerCollisionCheckSphereMove* mSphereMove;
    bool mIsEnableStandUp;
    bool mIsAboveGround;
    u8 _52[2];
    f32 mGroundHeight;
    sead::Vector3f mFallStartPos;
    f32 mFallDistance;
};

static_assert(sizeof(PlayerCollider2D3D) == 0x68);
