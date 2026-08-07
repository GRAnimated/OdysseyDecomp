#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Player/IJudge.h"

namespace al {
class CollisionParts;
class LiveActor;
}  // namespace al
class IPlayerModelChanger;
class IUsePlayerCollision;
class PlayerCarryKeeper;
class PlayerConst;
class PlayerExternalVelocity;
class PlayerInput;
class PlayerTrigger;

class PlayerJudgePoleClimb : public IJudge {
public:
    PlayerJudgePoleClimb(const al::LiveActor* player, const PlayerConst* pConst,
                         const IUsePlayerCollision* collision,
                         const IPlayerModelChanger* modelChanger,
                         const PlayerCarryKeeper* carryKeeper,
                         const PlayerExternalVelocity* externalVelocity,
                         const PlayerInput* input, const PlayerTrigger* trigger);

    void reset() override;
    void update() override;
    bool judge() const override;

    const al::CollisionParts* getCollidedParts() const { return mCollidedParts; }
    const sead::Vector3f& getPosition() const { return mPosition; }
    const sead::Vector3f& getUp() const { return mUp; }
    const sead::Vector3f& getFront() const { return mFront; }
    f32 getAngleOffsetWall() const { return mAngleOffsetWall; }
    f32 getPoleHeight() const { return mPoleHeight; }
    const char* getCollisionCode() const { return mCollisionCode; }
    const al::CollisionParts* getCollisionParts() const { return mCollidedParts; }
    const sead::Vector3f& getContactPos() const { return mPosition; }
    const sead::Vector3f& getPoleUp() const { return mUp; }
    const sead::Vector3f& getPoleFront() const { return mFront; }
    f32 getAngleOffset() const { return mAngleOffsetWall; }
    f32 getStartHeight() const { return mPoleHeight; }
    const char* getPoleCode() const { return mCollisionCode; }

private:
    const al::LiveActor* mPlayer;
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollision;
    const IPlayerModelChanger* mModelChanger;
    const PlayerCarryKeeper* mCarryKeeper;
    const PlayerExternalVelocity* mExternalVelocity;
    const PlayerInput* mInput;
    const PlayerTrigger* mTrigger;
    bool mIsJudge = false;
    const al::CollisionParts* mCollidedParts = nullptr;
    sead::Vector3f mPosition = sead::Vector3f::zero;
    sead::Vector3f mUp = sead::Vector3f::zero;
    sead::Vector3f mFront = sead::Vector3f::zero;
    f32 mAngleOffsetWall = 0.0f;
    f32 mPoleHeight = 0.0f;
    const char* mCollisionCode = "NoCollide";
};

static_assert(sizeof(PlayerJudgePoleClimb) == 0x90);
