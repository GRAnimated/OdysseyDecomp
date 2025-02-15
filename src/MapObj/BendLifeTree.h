#pragma once

#include "Library/LiveActor/LiveActor.h"

class BendLeafTree : public al::LiveActor {
public:
    BendLeafTree(const char* name);
    void init(const al::ActorInitInfo& info) override;
    bool initBendInfo();
    void initAfterPlacement() override;
    void exeWait();
    void movement() override;
    void control() override;
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                    al::HitSensor* self) override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;

private:
    s32 mNumJoints = 0;
    f32 mBendLength = 0.0f;
    f32* mBendSpeeds = nullptr;
    f32* mBendAngles = nullptr;
    s32* mDitherTimers = nullptr;
    s32* mBendTimers = nullptr;
    f32* mBendImpulses = nullptr;
    sead::Vector3f mLeafAPos = sead::Vector3f::zero;
    sead::Vector3f mClippingExpansion = sead::Vector3f::zero;
    f32 mScaleY = 1.0f;
};
