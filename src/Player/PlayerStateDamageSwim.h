#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
class WaterSurfaceFinder;
}  // namespace al
class IJudge;
class IUsePlayerCollision;
class PlayerAnimator;
class PlayerActionAirMoveControl;
class PlayerConst;
class PlayerEffect;
class PlayerInput;
class PlayerTrigger;

class PlayerStateDamageSwim : public al::ActorStateBase {
public:
    PlayerStateDamageSwim(al::LiveActor* player, const PlayerConst* pConst,
                          const IUsePlayerCollision* collision, const PlayerInput* input,
                          const PlayerTrigger* trigger, PlayerAnimator* animator,
                          const al::WaterSurfaceFinder* waterSurfaceFinder, PlayerEffect* effect,
                          IJudge* judgeInWater, IJudge* judgeOutInWater);
    ~PlayerStateDamageSwim() override;

    void appear() override;
    bool tryReactionWaterIn();
    bool tryReactionWaterOut();
    bool isReduceOxygen() const;
    bool isNoDamageDown() const;
    bool isEnableCancel() const;
    bool isEndGround() const;
    bool isEndInWater() const;
    void exeDamageSwim();
    void exeDamageSurface();
    void exeDamageLandWater();
    void exeDead();
    void exeEndGround();
    void exeEndOutOfWater();

private:
    const PlayerConst* mConst;
    const IUsePlayerCollision* mCollision;
    const PlayerTrigger* mTrigger;
    PlayerAnimator* mAnimator;
    const al::WaterSurfaceFinder* mWaterSurfaceFinder;
    PlayerEffect* mEffect;
    PlayerActionAirMoveControl* mAirMoveControl;
    IJudge* mJudgeInWater;
    IJudge* mJudgeOutInWater;
    bool _68;
    bool _69;
    bool mIsNoDamageDown;
    u8 _6b[5];
};

static_assert(sizeof(PlayerStateDamageSwim) == 0x70);
