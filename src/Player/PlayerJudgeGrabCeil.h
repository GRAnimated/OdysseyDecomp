#pragma once

#include <math/seadVector.h>

#include "Library/HostIO/HioNode.h"

#include "Player/IJudge.h"

namespace al {
class LiveActor;
class CollisionParts;
}  // namespace al
class PlayerConst;
class IUsePlayerCollision;
class IPlayerModelChanger;
class PlayerCarryKeeper;
class PlayerExternalVelocity;

class PlayerJudgeGrabCeil : public al::HioNode, public IJudge {
public:
    PlayerJudgeGrabCeil(const al::LiveActor* player, const PlayerConst* pConst,
                        const IUsePlayerCollision* collider,
                        const IPlayerModelChanger* modelChanger,
                        const PlayerCarryKeeper* carryKeeper,
                        const PlayerExternalVelocity* externalVelocity);
    void reset() override;
    void update() override;
    bool judge() const override;

    const al::CollisionParts* getCollidedParts() const { return mCollidedParts; }
    const sead::Vector3f& getPosition() const { return _48; }
    const sead::Vector3f& getUp() const { return _54; }
    const sead::Vector3f& getFront() const { return _60; }
    const al::CollisionParts* getCollisionParts() const { return mCollidedParts; }
    const sead::Vector3f& getContactPos() const { return _48; }
    const sead::Vector3f& getGrabUp() const { return _54; }
    const sead::Vector3f& getGrabFront() const { return _60; }

private:
    const al::LiveActor* mPlayer;
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollision;
    const IPlayerModelChanger* mModelChanger;
    const PlayerCarryKeeper* mCarryKeeper;
    const PlayerExternalVelocity* mExternalVelocity;
    bool mIsJudge = false;
    const al::CollisionParts* mCollidedParts = nullptr;
    sead::Vector3f _48 = {0.0f, 0.0f, 0.0f};
    sead::Vector3f _54 = {0.0f, 0.0f, 0.0f};
    sead::Vector3f _60 = {0.0f, 0.0f, 0.0f};
};

static_assert(sizeof(PlayerJudgeGrabCeil) == 0x70);
