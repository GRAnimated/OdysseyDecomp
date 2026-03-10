#include "Npc/SphinxRideStateRevival.h"

#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"

#include "Npc/SphinxRide.h"

namespace {
NERVE_IMPL(SphinxRideStateRevival, Revival);
NERVES_MAKE_NOSTRUCT(SphinxRideStateRevival, Revival);
}  // namespace

SphinxRideStateRevival::SphinxRideStateRevival(SphinxRide* host, const al::ActorInitInfo& initInfo,
                                               bool isFromTrans)
    : HostStateBase<SphinxRide>("復活状態(スフィンクス)", host) {
    initNerve(&Revival, 0);
    if (isFromTrans) {
        sead::Vector3f trans;
        sead::Vector3f rotate;
        al::tryGetTrans(&trans, initInfo);
        al::tryGetRotate(&rotate, initInfo);
        mRevivalTrans = trans;
        mRevivalRotate = rotate;
    } else {
        al::tryGetLinksTR(&mRevivalTrans, &mRevivalRotate, initInfo, "SphinxRevivalPoint");
    }
}

void SphinxRideStateRevival::appear() {
    al::NerveStateBase::appear();
    al::invalidateClipping(getHost());
    getHost()->disappear();
    al::offCollide(getHost());
    al::setNerve(this, &Revival);
}

void SphinxRideStateRevival::exeRevival() {
    if (al::isGreaterEqualStep(this, 120)) {
        al::setVelocityZero(getHost());
        al::resetRotatePosition(getHost(), mRevivalRotate, mRevivalTrans);
        al::onCollide(getHost());
        al::validateClipping(getHost());
        al::showModelIfHide(getHost());
        kill();
    }
}

bool SphinxRideStateRevival::isNoMove() {
    if (!al::isNear(al::getRotate(getHost()), mRevivalRotate, 0.001f))
        return false;
    return al::isNear(al::getTrans(getHost()), mRevivalTrans, 10.0f);
}
