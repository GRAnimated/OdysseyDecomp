#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Player/IUsePlayerCeilingCheck.h"
#include "Player/IUsePlayerCollision.h"
#include "Player/IUsePlayerHeightCheck.h"

namespace al {
class LiveActor;
}

class CollisionShapeKeeper;
class PlayerCeilingCheck;
class PlayerCollider;

class PlayerColliderYoshi : public IUsePlayerCollision,
                            public IUsePlayerCeilingCheck,
                            public IUsePlayerHeightCheck {
public:
    PlayerColliderYoshi(al::LiveActor* actor);

    void init();
    void tryChangeCollisionNormal();
    void updateCollider(const sead::Vector3f& velocity);
    void updateHeightCheck(const sead::Vector3f& position, const sead::Vector3f& gravity);
    void updateCeilingCheck(const sead::Vector3f& position, const sead::Vector3f& gravity,
                            f32 height);
    void tryChangeCollisionTongueJump();

    bool isEnableStandUp() const override;
    bool isEnableHoldUp() const override;
    bool isPressedCeil() const override;
    f32 getSafetyCeilSpace() const override;
    f32 getCeilCheckHeight() const override;
    PlayerCollider* getPlayerCollider() const override;
    bool isAboveGround() const override;
    f32 getGroundHeight() const override;
    f32 getShadowDropHeight() const override;

private:
    al::LiveActor* mActor;
    PlayerCollider* mPlayerCollider = nullptr;
    PlayerCeilingCheck* mCeilingCheck = nullptr;
    CollisionShapeKeeper* mCurrentCollisionShape = nullptr;
    CollisionShapeKeeper* mCollisionNormal = nullptr;
    CollisionShapeKeeper* mCollisionTongueJump = nullptr;
    bool mIsAboveGround = false;
    u8 _49[3];
    f32 mGroundHeight = 0.0f;
    f32 mShadowDropHeight = 0.0f;
};

static_assert(sizeof(PlayerColliderYoshi) == 0x58);
