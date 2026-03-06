#pragma once

#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

class NpcStateReaction;

class Tiara : public al::LiveActor {
public:
    Tiara(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) override;

    void tryStartScare();
    void tryEndScare();
    void startWait();
    void startTalk();
    void startStop();
    void startTurnToHostAndStop(s32 frames);
    void startStopAction(const char* actionName);
    void startFindTurn();
    bool isEndTurn() const;
    void startPeachWorldHomeCastleCapDemo();
    void startShakeHandPeachWorldHomeCastleCapDemo();
    void endPeachWorldHomeCastleCapDemo();

    void exeWait();
    void exeTalk();
    void exeStop();
    void endStop();
    void exeReaction();
    void exeTurnToHost();
    void exeStopToHost();
    void exeStopAction();
    void exeFindTurn();
    void exeShakeHand();
    void exeScare();
    void exeWaitInitPeachWorldHomeCastleCapDemo();
    void exeShakeHandPeachWorldHomeCastleCapDemo();
    void exeWaitAfterPeachWorldHomeCastleCapDemo();

    al::LiveActor* mHostActor = nullptr;
    NpcStateReaction* mNpcStateReaction = nullptr;
    const char* mWaitActionName = "Wait";
    sead::Vector3f mFrontDir = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mTurnDir = {0.0f, 0.0f, 0.0f};
    s32 mTurnStep = -1;
    bool mIsFindTurn = false;
    bool mIsInvalidTrample = false;
};

static_assert(sizeof(Tiara) == 0x140);
