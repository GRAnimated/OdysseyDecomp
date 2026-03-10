#pragma once

#include "Library/Nerve/NerveStateBase.h"

#include "Npc/YukimaruInput.h"

class IUsePlayerHack;
class PlayerHackStartShaderCtrl;
class Yukimaru;
class YukimaruStateMove;

namespace al {
class HitSensor;
class SensorMsg;
}  // namespace al

class YukimaruStateHack : public al::ActorStateBase, public YukimaruInput {
public:
    YukimaruStateHack(Yukimaru* yukimaru);

    void start(al::HitSensor* other, al::HitSensor* self, bool isRunning);
    bool tryStartDirect(const al::SensorMsg* msg);
    void onHackShadowAndSilhouette();
    void appear() override;
    void kill() override;
    void offHackShadowAndSilhouette();
    void control() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other);
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self);
    void exeWaitEnterMario();
    void exeHackStart();
    void exeMove();
    void exeCancel();
    void exeDamage();
    void exeKill();
    void exeTalkDemo();
    bool isEndCancel() const;
    bool isEndDamage() const;
    bool isEndKill() const;
    void updateMoveNoInput();
    void updateScale();
    void resetMoveEffect();
    bool isTriggerJump() const override;
    bool isHoldJump() const override;
    void calcInputVec(sead::Vector3f* out) const override;

    friend class Yukimaru;

private:
    IUsePlayerHack* mHackActor = nullptr;
    YukimaruStateMove* mStateMove = nullptr;
    PlayerHackStartShaderCtrl* mShaderCtrl = nullptr;
    bool mIsRunning = false;
};

static_assert(sizeof(YukimaruStateHack) == 0x48);
