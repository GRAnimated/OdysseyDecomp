#pragma once

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/Event/IEventFlowQueryJudge.h"
#include "Library/LiveActor/LiveActor.h"

class Doshi;

class ShoppingWatcher : public al::LiveActor,
                        public al::IEventFlowEventReceiver,
                        public al::IEventFlowQueryJudge {
public:
    ShoppingWatcher(const char*, const char*, Doshi*);
    void init(const al::ActorInitInfo&) override;
    bool isAfterBuyWear() const;
    bool isAfterBuyShine() const;
    bool isAfterBuyLifeUpItem() const;
    void initAfterPlacement() override;
    void tryStartCameraAfterBuyItem();
    void control() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                    al::HitSensor* self) override;
    bool receiveEvent(const al::EventFlowEventData*) override;
    void judgeQuery(const char*) const;
    void exeWait();
    void exeScare();
    void exeShop();
    void exeDemoWear();
    void tryEndCameraAfterBuyItem();
    void exeDemoGetShine();
    void exeDemoGetLifeUpItem();
    void exeEnd();
    void exeReaction();
    void exeTurnToInitFront();
    void exeTimeBalloonOrRace();
    bool isAliveNpcInShop() const;
    bool isWait() const;
    bool isShop() const;
    void endShop();
    void appearNpcInShop();
    void killNpcInShop();
    void getDoshiPos();
    void calcDoshiFrontDir(sead::Vector3f*);
    bool isInsideTerritoryPlayer() const;
    void requestStartDoshiCamera();
    void requestEndDoshiCamera();

    bool get_1C0() const { return _1C0; }

    const char* getShopName() const { return mShopName; }

private:
    void* filler[22];
    bool _1C0;
    void* _1C8;
    const char* mShopName;
    void* filler2[22];
};
