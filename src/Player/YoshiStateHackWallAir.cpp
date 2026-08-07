#include "Player/YoshiStateHackWallAir.h"

#include "Library/Nerve/Nerve.h"
#include "Library/Nerve/NerveKeeper.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerWallActionHistory.h"
#include "Player/YoshiStateHackWallCling.h"

namespace {
class YoshiStateHackWallAirNrvCling : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override {
        keeper->getParent<YoshiStateHackWallAir>()->exeCling();
    }
};

class YoshiStateHackWallAirNrvJump : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override {
        YoshiStateHackWallAir* state = keeper->getParent<YoshiStateHackWallAir>();
        if (al::updateNerveState(state))
            state->kill();
    }
};

NERVES_MAKE_STRUCT(YoshiStateHackWallAir, Cling, Jump)
}  // namespace

void YoshiStateHackWallAir::appear() {
    HackerStateBase::appear();
    mWallActionHistory->reset();
    al::setNerve(this, &NrvYoshiStateHackWallAir.Cling);
}

bool YoshiStateHackWallAir::isCling() const {
    return al::isNerve(this, &NrvYoshiStateHackWallAir.Cling);
}

bool YoshiStateHackWallAir::isAir() const {
    if (!al::isNerve(this, &NrvYoshiStateHackWallAir.Jump))
        return false;
    return al::isGreaterStep(this, 0);
}

void YoshiStateHackWallAir::setupCling(const al::CollisionParts* collisionParts,
                                       const sead::Vector3f& position,
                                       const sead::Vector3f& normal) {
    mStateCling->setup(collisionParts, position, normal);
}
