#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class HitSensor;
class SensorMsg;
}  // namespace al

class ActorStateReactionBase;
class DigPoint;
class Shibaken;
class ShibakenMoveAnimCtrl;
class ShibakenStateJump;
class ShibakenStateTurn;
class ShibakenStateWait;

class ShibakenStatePointChase : public al::HostStateBase<Shibaken> {
public:
    ShibakenStatePointChase(const char* name, Shibaken* shibaken,
                            ShibakenMoveAnimCtrl* moveAnimCtrl, ActorStateReactionBase* reaction);

    void appear() override;
    void kill() override;

    void startFirstWait(DigPoint* point);
    void startChaseRun(DigPoint* point);
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self);
    bool isKillByDeathArea() const;

    void exeFirstWait();
    void exeFindTurn();
    void exeFind();
    void exePointTurn();
    void exeChaseRun();
    void exeChaseWalk();
    void exeChaseWalkSniff();
    void exeChaseWalkSniffNear();
    void exeChaseFind();
    void exeSniff();
    void exeJump();
    void exeReaction();

private:
    void afterSubState(DigPoint* point, sead::Vector3f* offset);

    DigPoint* mCurrentPoint = nullptr;
    ShibakenMoveAnimCtrl* mMoveAnimCtrl;
    ShibakenStateWait* mStateWait = nullptr;
    ShibakenStateTurn* mStateTurn = nullptr;
    ShibakenStateJump* mStateJump = nullptr;
    ActorStateReactionBase* mReaction;
    sead::Quatf _50 = sead::Quatf::unit;
    sead::Quatf _60 = sead::Quatf::unit;
    sead::Vector3f _70 = {0, 0, 0};
    s32 _7c = 0;
};

static_assert(sizeof(ShibakenStatePointChase) == 0x80);
