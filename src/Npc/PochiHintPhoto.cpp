#include "Npc/PochiHintPhoto.h"

#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
NERVE_IMPL(PochiHintPhoto, Wait);
NERVES_MAKE_NOSTRUCT(PochiHintPhoto, Wait);
}  // namespace

PochiHintPhoto::PochiHintPhoto(const char* name) : al::LiveActor(name) {}

void PochiHintPhoto::setIsLinkChild() {
    mIsLinkChild = true;
}

void PochiHintPhoto::init(const al::ActorInitInfo& initInfo) {
    al::initMapPartsActorWithArchiveName(this, initInfo, "YamashitaJFixParts071", nullptr);
    if (mIsLinkChild)
        al::initNerve(this, &Wait, 0);
    makeActorDead();
}

bool PochiHintPhoto::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                                al::HitSensor* self) {
    return false;
}

void PochiHintPhoto::exeWait() {}
