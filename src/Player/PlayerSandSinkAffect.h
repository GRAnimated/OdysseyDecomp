#pragma once

#include <math/seadVector.h>

namespace al {
class LiveActor;
}
class PlayerConst;
class PlayerInput;
class IUsePlayerCollision;
class PlayerEffect;

class PlayerSandSinkAffect {
public:
    PlayerSandSinkAffect(const al::LiveActor* player, const PlayerConst* pConst,
                         const PlayerInput* input, IUsePlayerCollision* collider,
                         PlayerEffect* effect);
    void clear();
    bool isSink() const;
    void update(bool);
    bool isSinkDeathHeight() const;
    void reduceVelocity(sead::Vector3f* velocity);
    bool isEnableCapThrow() const;
    f32 calcSandSinkDeathRate() const;
    f32 getSinkVelocity() const { return mSinkVelocity; }
    bool isSafe() const { return mIsSafe; }

private:
    const al::LiveActor* mPlayer;
    const PlayerConst* mConst;
    const PlayerInput* mInput;
    IUsePlayerCollision* mCollider;
    PlayerEffect* mEffect;
    f32 mSinkVelocity = 0.0f;
    f32 mSinkAmount = 0.0f;
    bool mIsSafe = false;
};

static_assert(sizeof(PlayerSandSinkAffect) == 0x38);
