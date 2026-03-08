#pragma once

#include <container/seadBuffer.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}

namespace sead {
template <typename T>
class PtrArray;
}

class HomeCapMovePoint;

class HomeCapStateMoveSouvenir : public al::ActorStateBase {
public:
    HomeCapStateMoveSouvenir(al::LiveActor* actor,
                             const sead::PtrArray<HomeCapMovePoint>& movePoints);

    void init() override;
    void appear() override;

    bool tryMoveSouvenir();
    void setReturnChair(al::LiveActor* returnChair);

    void exeTurnNextMovePoint();
    void exeWaitSouvenirAction();
    void exeMoveSouvenirFront();
    void exeTurnSouvenirDir();
    void exeWaitSouvenirFront();
    void exeActionSouvenirFront();
    void exeReturnChairTurn();
    void exeReturnChair();
    void exeTurnChairFront();

private:
    al::LiveActor* mReturnChair = nullptr;
    const sead::PtrArray<HomeCapMovePoint>* mMovePoints;
    s32 mMovePointIndex = 0;
    sead::Quatf mSavedQuat = sead::Quatf::unit;
    sead::Vector3f mSavedPos = sead::Vector3f::zero;
    sead::Buffer<bool> mTrackBuffer;
};

static_assert(sizeof(HomeCapStateMoveSouvenir) == 0x60);
