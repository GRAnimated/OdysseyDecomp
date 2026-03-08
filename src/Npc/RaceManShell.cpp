#include "Npc/RaceManShell.h"

#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"

#include "Util/SensorMsgFunction.h"

RaceManShell::RaceManShell(const char* name) : al::LiveActor(name) {}

void RaceManShell::init(const al::ActorInitInfo& initInfo) {
    al::initActorWithArchiveName(this, initInfo, "NokonokoShell", nullptr);
    al::invalidateClipping(this);
    makeActorDead();
}

void RaceManShell::control() {}

// NON_MATCHING: branch direction differences and tail call optimization for last check
bool RaceManShell::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) {
    if (!al::isSensorName(self, "Body"))
        return false;
    if (al::isMsgPlayerDisregard(msg))
        return false;
    if (rs::isMsgPlayerDisregardHomingAttack(msg))
        return true;
    if (rs::isMsgPlayerDisregardTargetMarker(msg))
        return true;
    return false;
}
