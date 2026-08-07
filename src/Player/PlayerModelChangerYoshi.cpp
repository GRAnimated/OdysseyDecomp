#include "Player/PlayerModelChangerYoshi.h"

#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/LiveActor/LiveActorFunction.h"

#include "Player/PlayerModelHolder.h"

PlayerModelChangerYoshi::PlayerModelChangerYoshi(const al::LiveActor* host,
                                                 PlayerModelHolder* modelHolder)
    : mHost(host), mModelHolder(modelHolder) {
    mModelActor = mModelHolder->getCurrentModelActor();
}

void PlayerModelChangerYoshi::syncHost() {
    if (mIsVisibilityNeedsSync) {
        syncModelFlag(mModelActor);
    }
    al::copyPose(mModelActor, mHost);
}

void PlayerModelChangerYoshi::syncModelFlag(al::LiveActor* modelActor) {
    if (mIsModelVisible) {
        if (al::isDead(modelActor))
            modelActor->makeActorAlive();
    } else if (al::isAlive(modelActor)) {
        modelActor->makeActorDead();
    }

    if (mIsSilhouetteVisible)
        al::showSilhouetteModel(modelActor);
    else
        al::hideSilhouetteModel(modelActor);
}

void PlayerModelChangerYoshi::appearModel() {
    mIsVisibilityNeedsSync = true;
    mIsModelVisible = true;
}

void PlayerModelChangerYoshi::killModel() {
    mIsVisibilityNeedsSync = true;
    mIsModelVisible = false;
}

void PlayerModelChangerYoshi::resetPosition() {
    al::copyPose(mModelActor, mHost);
    al::resetPosition(mModelActor);
    const s32 subActorNum = al::getSubActorNum(mModelActor);
    for (s32 i = 0; i < subActorNum; i++)
        al::getSubActor(mModelActor, i)->makeActorAlive();
}

void PlayerModelChangerYoshi::hideModel() {}

void PlayerModelChangerYoshi::hideSilhouette() {
    mIsVisibilityNeedsSync = true;
    mIsSilhouetteVisible = false;
}

void PlayerModelChangerYoshi::showModel() {}

void PlayerModelChangerYoshi::showSilhouette() {
    mIsVisibilityNeedsSync = true;
    mIsSilhouetteVisible = true;
}

bool PlayerModelChangerYoshi::isHiddenModel() const {
    return false;
}

void PlayerModelChangerYoshi::changeModel(al::LiveActor* modelActor) {
    if (al::isAlive(mModelActor))
        mModelActor->makeActorDead();
    syncModelFlag(modelActor);
    mModelActor = modelActor;
}

bool PlayerModelChangerYoshi::isFireFlower() const {
    return false;
}

bool PlayerModelChangerYoshi::isMini() const {
    return false;
}

bool PlayerModelChangerYoshi::isChange() const {
    return false;
}

bool PlayerModelChangerYoshi::is2DModel() const {
    return false;
}

bool PlayerModelChangerYoshi::isHiddenShadowMask() const {
    return false;
}

void PlayerModelChangerYoshi::hideShadowMask() {}

void PlayerModelChangerYoshi::showShadowMask() {}
