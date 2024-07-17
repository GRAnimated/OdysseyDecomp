#pragma once

#include "Library/LiveActor/LiveActor.h"

class KuriboMini : public al::LiveActor {
public:
    KuriboMini(const char* actorName);

    virtual void init(const al::ActorInitInfo& info) override;
    virtual void makeActorAlive() override;
    virtual void makeActorDead() override;
    virtual void appear() override;
    virtual void kill() override;
    virtual void appearPop();
    virtual void appearPopBack();
    virtual void control() override;
    void checkSandSinkPrecisely();
    virtual void updateCollider() override;
    virtual void startClipped() override;
    virtual void endClipped() override;
    void noRevive();
    void exeWait();
    void setShiftTypeOnGround(s32);
    bool isPlayerUp();
    void exeWander();
    bool tryShiftDrown();
    void exeTurn();
    void exeFind();
    void exeChaseReady();
    void exeChase();
    void exeStop();
    void exeAttack();
    void exeBlow();
    void exeBlowLand();
    void exeBlowRecover();
    void exePressDown();
    void exeBlowDown();
    void exeFall();
    void exeLand();
    bool tryShiftChaseOrWander();
    void exeSink();
    void updateSink();
    void exeReset();
    void exeSandGeyser();
    void exeDrown();
    void exeHide();
    void exePopAppearStart();
    void exePopAppear();
    void exePopAppearEnd();
    void exeBind();
    virtual void attackSensor(al::HitSensor* target, al::HitSensor* source) override;
    virtual bool receiveMsg(const al::SensorMsg* message, al::HitSensor* source,
                            al::HitSensor* target) override;
    void prepareKillByShineGet();
    bool tryReceiveMsgNormal(const al::SensorMsg* message, al::HitSensor* source,
                             al::HitSensor* target);
    void notifyJumpSink(f32);
    void validateSpecialPush(u32);
    void forceStartClipped();

private:
    void* filler[22];
};
