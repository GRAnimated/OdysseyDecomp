#include "Npc/RaceManThrowCap.h"

#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"

RaceManThrowCap::RaceManThrowCap(const char* name) : al::LiveActor(name) {}

void RaceManThrowCap::init(const al::ActorInitInfo& initInfo) {
    al::initActorWithArchiveName(this, initInfo, "RaceManCap", nullptr);
    al::initJointControllerKeeper(this, 1);
    al::initJointLocalYRotator(this, &mYRotation, "JointRoot");
    al::invalidateClipping(this);
    makeActorDead();
}

void RaceManThrowCap::control() {
    mYRotation += -30.0f;
}
