#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

namespace al {
class CollisionPartsConnector;
}

class ItemGenerator;

class Squirrel : public al::LiveActor {
public:
    Squirrel(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void initAfterPlacement() override;
    void control() override;
    void appear() override;
    void kill() override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) override;

    void exeHide();
    void exeAppear();
    void exeWait();
    void exeTurn();
    void exeRunaway();
    void exeDisappear();
    void exeRestartPrepare();

private:
    f32 mFindDistance = 1750.0f;
    f32 mTurnSpeed = 12.5f;
    f32 mRunAccel = 2.0f;
    f32 mFriction = 0.9f;
    s32 mMoveStep = 75;
    f32 mRandomWaitProbA = 0.3f;
    f32 mRandomWaitProbB = 0.3f;
    bool mIsValidReflectWall = false;
    bool mIsTurnOnPoint = false;
    u8 _126[2]{};
    bool mIsExistTurnLR = false;
    bool mIsExistWaitAB = false;
    bool mIsExistWaitRandom = false;
    bool mIsExistWaitRandomAB = false;
    bool _12c = false;
    u8 _12d[3]{};
    al::CollisionPartsConnector* mConnector = nullptr;
    ItemGenerator* mItemGenerator = nullptr;
    sead::Vector3f mInitTrans = {};
    sead::Quatf mInitQuat = sead::Quatf::unit;
    s32 mRunawayCount = 1;
    s32 mRunawayCounter = 0;
    s32 mRestartFrame = 0;
    sead::Vector3f mRunawayDir = {};
    bool mIsHide = false;
    bool mIsLeftRight = false;
};

static_assert(sizeof(Squirrel) == 0x178);
