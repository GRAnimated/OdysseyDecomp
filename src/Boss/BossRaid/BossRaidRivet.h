#pragma once

#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

namespace al {
class MtxConnector;
}

class BossRaidChainList;
class CapTargetInfo;
class ItemGenerator;
class Popn;

class BossRaidRivet : public al::LiveActor {
public:
    BossRaidRivet(const char* name);
    void init(const al::ActorInitInfo& info) override;
    void calcAnim() override;
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                    al::HitSensor* self) override;

    void setConnect(al::LiveActor* actor, const char* jointName, const sead::Vector3f& rotate,
                    const sead::Vector3f& trans);
    void setChainRootConnect(al::LiveActor* actor, const char* jointName,
                             const sead::Vector3f& rotate, const sead::Vector3f& trans);
    void createChainAndPopn(al::LiveActor* actor, const al::ActorInitInfo& info);
    bool isElectric() const;
    bool isEnableCapKeepLockOn() const;
    bool isPullOut() const;

    void exeDemo();
    void exeWait();
    void exeElectricSign();
    void exeElectric();
    void exeElectricEnd();
    void exePull();
    void exePullOut();

    void appearPopn();
    void reset();
    void startAnim(const char* animName);
    void startElectricSign();
    void startElectric();
    void endElectric();
    void tryAppearPopn();
    void tryKillPopn();
    void killChain();
    void resetChain();

private:
    CapTargetInfo* mCapTargetInfo = nullptr;
    al::MtxConnector* mMtxConnector = nullptr;
    al::MtxConnector* mChainRootMtxConnector = nullptr;
    BossRaidChainList* mChainList = nullptr;
    Popn* mPopn = nullptr;
    ItemGenerator* mItemGenerator = nullptr;
    sead::Vector3f mChainRootPos = sead::Vector3f::zero;
    sead::Vector3f mCapPointPos = sead::Vector3f::zero;
    f32 mJointAngle = 0.0f;
    bool mIsPopnKilled = false;
};

static_assert(sizeof(BossRaidRivet) == 0x158);
