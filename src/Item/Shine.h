#pragma once

#include "Library/LiveActor/LiveActor.h"

class Shine : public al::LiveActor {
public:
    Shine(const char* actorName);
    virtual void init(const al::ActorInitInfo& info) override;
    void getCurrentModel();
    void tryExpandShadowAndClipping();
    void initAppearDemo(const al::ActorInitInfo& info);
    void onAppear();
    void offAppear();
    void hideAllModel();
    void invalidateKillSensor();
    virtual void initAfterPlacement() override;
    void getDirect();
    void updateHintTrans(const sead::Vector3f&) const;
    virtual void appear() override;
    virtual void makeActorAlive() override;
    virtual void makeActorDead() override;
    virtual void control() override;
    void updateModelActorPose();
    virtual void attackSensor(al::HitSensor* target, al::HitSensor* source) override;
    virtual bool receiveMsg(const al::SensorMsg* message, al::HitSensor* source,
                            al::HitSensor* target) override;
    void showCurrentModel();
    void appearPopup();
    void addDemoActorWithModel();
    void get();
    virtual void endClipped() override;
    void initAppearDemoFromHost(const al::ActorInitInfo& info, const sead::Vector3f&);
    void initAppearDemoFromHost(const al::ActorInitInfo& info);
    void initAppearDemoActorCamera(const al::ActorInitInfo& info);
    void createShineEffectInsideObject(const al::ActorInitInfo& info, const sead::Vector3f&,
                                       const char*);
    bool isGot() const;
    bool isEmptyShineForDemoGetGrand() const;
    void setShopShine();
    bool isEndAppear() const;
    bool isEndAppearGK() const;
    void onSwitchGet();
    void getColorFrame() const;
    void setHintPhotoShine(const al::ActorInitInfo& info);
    void appearCommon();
    void tryChangeCoin();
    void tryAppearOrDemoAppear();
    void appearPopup(const sead::Vector3f&);
    void appearPopupDelay(int);
    void appearPopupSlot(const sead::Vector3f&);
    void appearWarp(const sead::Vector3f&, const sead::Vector3f&);
    void appearStatic();
    void appearPopupWithoutDemo();
    void appearPopupGrandByBoss(int);
    void appearPopupWithoutWarp();
    void appearAndJoinBossDemo(const char*, const sead::Quatf&, const sead::Vector3f&);
    void endBossDemo();
    void endBossDemoAndStartFall(f32);
    void appearWait();
    void appearWait(const sead::Vector3f&);
    void startHold();
    void startFall();
    void getDirectWithDemo();
    void addDemoModelActor();
    void setGrandShine();
    void exeWaitRequestDemo();
    void exeWaitKill();
    void exeDemoAppear();
    void tryWaitCameraInterpole() const;
    void tryStartAppearDemo();
    void calcCameraAt();
    void exeDemoMove();
    void updateIgnoreFrame();
    void exeDemoWait();
    void exeDemoGet();
    void exeDemoGetMain();
    void exeDemoGetGrand();
    void exeBossDemo();
    void exeBossDemoAfterFall();
    void exeBossDemoAfterLanding();
    void exeBossDemoFall();
    void exeBossDemoFallSlowdown();
    void exeBossDemoRise();
    void exeBossDemoRiseDamp();
    void exeAppearSlot();
    void exeAppearSlotDown();
    void exeAppear();
    void exeAppearWait();
    void exeAppearDown();
    void exeAppearStatic();
    void exeAppearEnd();
    void exeAppearWaitCameraInterpole();
    void exeWait();
    void exeGot();
    void exeHold();
    void exeFall();
    void exeDelay();
    void exeHide();
    void exeReaction();
    void exeCoin();
    void updateModelActorResetPosition();

private:
    void* filler[79];
};

namespace ShineFunction {
const char* getMovePointLinkName();
}