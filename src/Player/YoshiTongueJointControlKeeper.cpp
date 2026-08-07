#include "Player/YoshiTongueJointControlKeeper.h"

#include "Library/Joint/JointControllerKeeper.h"

#include "Player/YoshiTongueJointControlStretch.h"

YoshiTongueJointControlKeeper::YoshiTongueJointControlKeeper(const al::LiveActor* actor,
                                                             const al::LiveActor* modelActor)
    : mActor(actor), mModelActor(modelActor) {
    al::initJointControllerKeeper(actor, 1);
    mStretch = new YoshiTongueJointControlStretch(mActor);
    al::registerJointController(mActor, mStretch);
}

void YoshiTongueJointControlKeeper::update(const sead::Vector3f& rootPos,
                                           const sead::Vector3f& direction,
                                           const sead::Vector3f& tipPos) {
    mStretch->update(rootPos, direction, tipPos);
}

void YoshiTongueJointControlKeeper::calcTongueBoundingBox(sead::BoundBox3f* boundingBox) const {
    mStretch->calcBoundingBox(boundingBox);
}
