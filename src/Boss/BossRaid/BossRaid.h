#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/LiveActor/LiveActor.h"

#include "Demo/IUseDemoSkip.h"

namespace al {
class ByamlIter;
class CameraTicket;
class LiveActorGroup;
class MtxConnector;
class ParabolicPath;
struct ActorInitInfo;
}  // namespace al

class BossRaidStateBreathAttack;
class BossRaidStateGroundAttack;
class BossStateTalkDemo;
class Shine;

class BossRaid : public al::LiveActor, public IUseDemoSkip, public al::IEventFlowEventReceiver {
public:
    BossRaid(const char* name);
    void init(const al::ActorInitInfo& info) override;

    void initRivetList(const al::ActorInitInfo& info);
    void killRivetAll();
    void invalidateCollisionAll();
    void startDemoBattleStart();
    void startBattle();
    al::LiveActor* tryGetFollowTargetInfo(al::LiveActor** actor, sead::Vector3f* trans,
                                          sead::Vector3f* rotate, const char** jointName,
                                          const al::ByamlIter& iter);
    void endBattleStartDemo();
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool isElectric() const;
    bool receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                    al::HitSensor* self) override;
    bool isEnableDamage() const;
    bool receiveEvent(const al::EventFlowEventData* event) override;
    void calcAnim() override;
    void updatePlayerPose();
    void control() override;
    bool isPullOutRivetAll() const;
    void hintCapMessage(const char* message, s32 unused);
    bool isPullOutRivetAny() const;
    bool isNearWeakPoint() const;
    void damageCapMessage();
    bool isFirstDemo() const override;
    bool isEnableSkipDemo() const override;
    void skipDemo() override;
    void exeBattleStartWait();
    void exePreDemoBattleStart();
    void setUpDemoBattleStart();
    void exeDemoBattleStart();
    void startActionMain(const char* actionName);
    s32 getEnableRivetCount() const;
    void resetChainAll();
    void exeStartAttack();
    void exeBreathAttack();
    void exeGroundAttack();
    void exeTired();
    void appearRivetPopnAll();
    void exeUpSign();
    void startElectricSignParts();
    void exeUp();
    void killRivetPopnAll();
    void startAttack();
    void exeDamage();
    void exeRoarSign();
    void exeRoar();
    void resetRivetAll();
    void startRoarAnimParts();
    void exePreDemoBattleEnd();
    void setUpDemoBattleEnd();
    void exeDemoBattleEnd();
    void exeEndTalk();
    s32 getShotLevel() const;
    void startElectricParts();
    void endElectricParts();
    void validateCollisionAll();
    void killElectoricAll();
    void killChainAll();
    void appearAndHideRivetAll();
    void showRivetAll();
    void startDemoBattleEnd();

    friend class BossRaidStateGroundAttack;

private:
    al::LiveActorGroup* mRivetList = nullptr;
    al::LiveActor* mArmorActor = nullptr;
    al::LiveActor* mArmorBodyActor = nullptr;
    al::LiveActor* mArmorBrokenActor = nullptr;
    al::CameraTicket* mDemoCamera = nullptr;
    Shine* mShineActor = nullptr;
    al::ParabolicPath* mParabolicPath = nullptr;
    BossStateTalkDemo* mStateTalkDemo = nullptr;
    BossRaidStateGroundAttack* mStateGroundAttack = nullptr;
    BossRaidStateBreathAttack* mStateBreathAttack = nullptr;
    al::MtxConnector* mMtxConnector = nullptr;
    sead::Quatf mInitQuat = sead::Quatf::unit;
    sead::Vector3f mInitTrans = sead::Vector3f::zero;
    s32 mPhase = 3;
    s32 mLevel = 1;
    s32 mDamageCount = 0;
    bool mHasShownHintElectricSign = false;
    bool mHasShownHintRivet = false;
    s32 mHintWeakPointThreshold = -1;
    bool mHasShownHintHipDrop = false;
    bool mHasShownHintLast = false;
};

static_assert(sizeof(BossRaid) == 0x1A8);
