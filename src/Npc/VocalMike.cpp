#include "Npc/VocalMike.h"

#include "Library/LiveActor/ActorInitInfo.h"
#include "Library/LiveActor/ActorSensorMsgFunction.h"

#include "Util/Sensor.h"

VocalMike::VocalMike(const char* actorName) : al::LiveActor(actorName) {}

void VocalMike::init(const al::ActorInitInfo& info) {
    al::initActor(this, info);
    makeActorAlive();
}

bool VocalMike::receiveMsg(const al::SensorMsg* message, al::HitSensor* source,
                           al::HitSensor* target) {
    if (al::isMsgPlayerDisregard(message) || rs::isMsgPlayerDisregardHomingAttack(message))
        return true;
    return rs::isMsgPlayerDisregardTargetMarker(message);
}