#include "Npc/RaceAudienceNpc.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorInitInfo.h"
#include "Library/Math/MathRandomUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"

namespace {
NERVE_IMPL(RaceAudienceNpc, Wait);
NERVE_IMPL(RaceAudienceNpc, Dance);
NERVE_IMPL(RaceAudienceNpc, Jump);
NERVE_IMPL(RaceAudienceNpc, DanceRandom);

NERVES_MAKE_STRUCT(RaceAudienceNpc, Wait, Dance, Jump, DanceRandom);
}  // namespace

RaceAudienceNpc::RaceAudienceNpc(const char* actorName) : al::LiveActor(actorName) {}

void RaceAudienceNpc::init(const al::ActorInitInfo& info) {
    al::initActor(this, info);
    al::getArg(&field_108, info, "AudienceActionType");
    switch (field_108) {
    case 0:
        al::initNerve(this, &NrvRaceAudienceNpc.Wait, 0);
        break;
    case 1:
        al::initNerve(this, &NrvRaceAudienceNpc.Dance, 0);
        break;
    case 2:
        al::initNerve(this, &NrvRaceAudienceNpc.Jump, 0);
        break;
    }
    makeActorAlive();
}

void RaceAudienceNpc::exeWait() {
    if (al::isFirstStep(this))
        al::startAction(this, "WaitHappy");
}

void RaceAudienceNpc::exeDance() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Excited");
        field_110 = al::getRandom(600) + 600;
    }
    if (al::isGreaterEqualStep(this, field_110)) {
        if (field_108 == 2)
            al::setNerve(this, &NrvRaceAudienceNpc.Jump);
        else
            al::setNerve(this, &NrvRaceAudienceNpc.DanceRandom);
    }
}

void RaceAudienceNpc::exeDanceRandom() {
    if (al::isFirstStep(this))
        al::startAction(this, "ExcitedRandom");
    if (al::isActionEnd(this))
        al::setNerve(this, &NrvRaceAudienceNpc.Dance);
}

void RaceAudienceNpc::exeJump() {
    if (al::isFirstStep(this))
        al::startAction(this, "ExcitedJump");
}
