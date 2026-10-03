#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
struct ActorInitInfo;
}  // namespace al

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
    enum class IceWaterLevel : s32 { None, Low, Medium, High };

    static void deleteIceEffect(al::LiveActor* iceEffect, IceWaterLevel level);

    al::LiveActor* mPlayer;
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollider;
    IJudge* mJudge;
    al::LiveActor* mIceEffect = nullptr;
    s32 mIceWaterCount = 0;
    s32 mRecoveryCount = 0;
    IceWaterLevel mIceWaterLevel = IceWaterLevel::None;
    bool mIsInIceWater = false;
    bool mIsShowCapMsg = false;
};

static_assert(sizeof(PlayerCounterIceWater) == 0x38);
