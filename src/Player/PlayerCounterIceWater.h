#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
struct ActorInitInfo;
}

class PlayerConst;
class IUsePlayerCollision;
class IJudge;

class PlayerCounterIceWater {
public:
    PlayerCounterIceWater(al::LiveActor* player, const al::ActorInitInfo& initInfo,
                          const PlayerConst* playerConst, const IUsePlayerCollision* collider,
                          IJudge* judge);

    void clearIceWaterCount();
    void updateCount(bool isInIceWater, bool isOnGround);
    bool isTriggerDamage() const;
    void updateRecoveryCountImpl();
    void killIceEffect();
    bool isInIceWater() const { return mIsInIceWater; }

private:
    al::LiveActor* mPlayer;
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollider;
    IJudge* mJudge;
    al::LiveActor* mIceEffect = nullptr;
    s32 mIceWaterCount = 0;
    s32 mRecoveryCount = 0;
    s32 mIceWaterLevel = 0;
    bool mIsInIceWater = false;
    bool mIsShowCapMsg = false;
};

static_assert(sizeof(PlayerCounterIceWater) == 0x38);
