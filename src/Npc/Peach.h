#pragma once

#include "Library/LiveActor/LiveActor.h"

namespace al {
class JointSpringControllerHolder;
}

class Peach : public al::LiveActor {
public:
    Peach(const char* name);

    void init(const al::ActorInitInfo& initInfo) override;
    void control() override;

    void makeActorDeadBouquet();
    void startWaitKoopaShip();
    void startWaitKoopaShipWithKoopa();
    void startWaitChurch();
    void startWaitDemo(bool disableSpring);
    void startDemoAction(const char* actionName);

    void exeWait();
    void exeWaitKoopaShip();
    void exeWaitKoopaShipWithKoopa();
    void exeWaitChurch();
    void exeWaitDemo();
    void endWaitDemo();

private:
    __attribute__((noinline)) void syncEyeSubActorAction(const char* fallbackAction);

    al::JointSpringControllerHolder* mJointSpringHolder = nullptr;
    al::JointSpringControllerHolder* mSubActorSpringHolder = nullptr;
    const char* mPreviousVisAnimName = nullptr;
};

static_assert(sizeof(Peach) == 0x120);
