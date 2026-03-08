#pragma once

#include <container/seadPtrArray.h>

#include "Library/LiveActor/LiveActor.h"

class HomeCapMovePoint;
class HomeCapStateMoveChair;
class HomeCapStateMoveSouvenir;
class HomeChair;

class HomeCap : public al::LiveActor {
public:
    HomeCap(const char* name, HomeChair* chair,
            const sead::PtrArray<HomeCapMovePoint>& movePoints);

    void init(const al::ActorInitInfo& initInfo) override;

    void appearMoveOtherChair(al::LiveActor* otherChair);
    bool tryKillReturnHead();

    void exeHide();
    void exeMoveOtherChair();
    void exeWaitOtherChair();
    void exeWanderSign();
    void exeMoveSouvenir();
    void exeSleepWait();
    void exeNoMoveSouvenirSleepWait();
    void exeSleepStart();
    void exeSleep();
    void exeSleepEnd();

private:
    HomeChair* mChair;
    HomeCapStateMoveChair* mMoveChairState;
    HomeCapStateMoveSouvenir* mMoveSouvenirState;
    const sead::PtrArray<HomeCapMovePoint>* mMovePoints;
};

static_assert(sizeof(HomeCap) == 0x128);
