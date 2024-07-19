#include "MapObj/PictureNoStageChange.h"
#include "Library/LiveActor/LiveActorUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Util/Sensor.h"

namespace {
NERVE_IMPL(PictureNoStageChange, Wait);
NERVE_MAKE(PictureNoStageChange, Wait);
}  // namespace

PictureNoStageChange::PictureNoStageChange(const char* actorName, const char* textureName)
    : al::LiveActor(actorName), mTextureName(textureName) {}

void PictureNoStageChange::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, mTextureName, nullptr);
    al::initNerve(this, &Wait, 0);
    makeActorAlive();
}

bool PictureNoStageChange::receiveMsg(const al::SensorMsg* message, al::HitSensor* source,
                                      al::HitSensor* target) {
    if (rs::isMsgPlayerDisregardHomingAttack(message))
        return true;
    return rs::isMsgPlayerDisregardTargetMarker(message);
}

void PictureNoStageChange::exeWait() {}
