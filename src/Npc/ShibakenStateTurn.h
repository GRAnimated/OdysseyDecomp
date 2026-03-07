#pragma once

#include <math/seadQuat.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}

class ShibakenStateTurn : public al::ActorStateBase {
public:
    ShibakenStateTurn(const char* name, al::LiveActor* actor);

    bool tryStartTurn(const sead::Vector3f& targetPos);

    void exeTurnL30();
    void exeTurnL90();
    void exeTurnL180();
    void exeTurnR30();
    void exeTurnR90();
    void exeTurnR180();

private:
    void exeTurn(const char* actionName, const sead::Quatf& from, const sead::Quatf& to);

    sead::Quatf _20 = sead::Quatf::unit;
    sead::Quatf _30 = sead::Quatf::unit;
};

static_assert(sizeof(ShibakenStateTurn) == 0x40);
