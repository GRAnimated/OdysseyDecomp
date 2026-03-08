#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

namespace al {
class CameraTicket;
class EventFlowExecutor;
class JointLookAtController;
}  // namespace al

class CapTargetInfo;
class IUsePlayerHack;
class NpcStateReaction;
class PlayerHackStartShaderCtrl;
class RadiconCar;
class RandomActionUpdater;
class TalkNpcActionAnimInfo;
class TalkNpcParam;

class RadiconNpc : public al::LiveActor {
public:
    RadiconNpc(const char* name);

    void init(const al::ActorInitInfo& initInfo) override;
    void initAfterPlacement() override;
    void kill() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                    al::HitSensor* self) override;
    void control() override;

    void endHack();
    void exeWait();
    void exeWaitHack();
    void exeStartHack();
    void exeHackWait();
    void exeHackShake();
    void exeEndHack();
    void exeReaction();

private:
    TalkNpcParam* mParam = nullptr;
    IUsePlayerHack* mPlayerHack = nullptr;
    PlayerHackStartShaderCtrl* mPlayerHackStartShaderCtrl = nullptr;
    CapTargetInfo* mCapTargetInfo = nullptr;
    al::CameraTicket* mCameraTicket = nullptr;
    RadiconCar* mRadiconCar = nullptr;
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    NpcStateReaction* mNpcStateReaction = nullptr;
    TalkNpcActionAnimInfo* mActionAnimInfo = nullptr;
    RandomActionUpdater* mRandomActionUpdater = nullptr;
    al::JointLookAtController* mJointLookAtController = nullptr;
    sead::Vector3f mCarTrans = {0.0f, 0.0f, 0.0f};
    sead::Quatf mInitQuat = sead::Quatf::unit;
    bool _17c = false;
};

static_assert(sizeof(RadiconNpc) == 0x180);
