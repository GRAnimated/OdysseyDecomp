#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

#include "Npc/YukimaruInput.h"

struct ActorInitInfo;

namespace al {
class HitSensor;
class SensorMsg;
}  // namespace al

class YukimaruStateMove;

class YukimaruRacerTiago : public al::LiveActor, public YukimaruInput {
public:
    YukimaruRacerTiago(const char* name);

    void init(const al::ActorInitInfo& initInfo) override;
    void initAfterPlacement() override;
    void movement() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) override;

    void start();
    void exeWait();
    void exeRun();
    void updateMoveVec();
    sead::Vector3f calcCorrection(const sead::Vector3f& direction);
    sead::Vector3f calcError(const sead::Vector3f& pos) const;
    sead::Vector3f calcMoveVector(const sead::Vector3f& input);
    f32 calcMaxPerturbation(bool* isBehind, f32* distance);
    void calcReactionToPlayer(f32 progressDiff);

    bool isTriggerJump() const override;
    bool isHoldJump() const override;
    void calcInputVec(sead::Vector3f* out) const override;

    void outputInfo();

private:
    sead::Quatf mRotation;
    YukimaruStateMove* mStateMove;
    f32 mRailErrorScale;
    f32* mBaseSpeedPerLap;
    s32 mNumLaps;
    s32 _13c;
    f32 mIntelligence;
    sead::Vector3f mMoveVec;
    sead::Vector3f mFilterError;
    sead::Vector3f mFilterPrev;
    sead::Vector3f mFilterOutput;
    s32 mUpdateInterval;
    f32 mReactionX;
    f32 mReactionZ;
    f32 mIdealDistanceFromPlayer;
    bool _184;
};

static_assert(sizeof(YukimaruRacerTiago) == 0x188);
