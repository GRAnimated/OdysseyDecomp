#include "Util/YoshiUtil.h"

#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/Collision/PartsInterpolator.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/MatrixUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Nature/WaterSurfaceFinder.h"

#include "Player/PlayerConst.h"
#include "Player/YoshiTongue.h"

namespace rs {
bool isInPuddleHeight(const al::WaterSurfaceFinder* surfaceFinder, const PlayerConst* playerConst) {
    return surfaceFinder->isFoundSurface() &&
           surfaceFinder->getDistance() < playerConst->getSwimCenterOffset();
}

bool isSensorTypeYoshiEnableSendPush(const al::HitSensor* sensor) {
    return al::isSensorNpc(sensor);
}

bool isSensorTypeYoshiMsgReceivable(const al::HitSensor* sensor) {
    return al::isSensorNpc(sensor);
}
// NON_MATCHING: clean SEAD form is target-sized (216 bytes), but Clang preserves W0 through a
// temporary after tryNormalizeOrZero where the target branches directly on W0; next hypothesis is
// the original boolean/control-flow expression that gives distinct lifetimes to the two predicates.
bool tryCalcTonguePullPose(sead::Quatf* pose, const al::LiveActor* actor,
                           const YoshiTongue* yoshiTongue) {
    sead::Vector3f pullDistance(0.0f, 0.0f, 0.0f);
    if (!yoshiTongue->tryCalcTonguePullDistance(&pullDistance))
        return false;
    sead::Vector3f front(0.0f, 0.0f, 0.0f);
    if (!al::tryNormalizeOrZero(&front, pullDistance))
        return false;
    sead::Vector3f up(0.0f, 0.0f, 0.0f);
    al::calcUpDir(&up, actor);
    if (al::isParallelDirection(up, front, 0.01f)) {
        al::calcFrontDir(&up, actor);
        up.negate();
    }
    al::makeQuatFrontUp(pose, front, up);
    return true;
}

bool findClingGroundPos(sead::Vector3f* pos, const al::LiveActor* actor,
                        const sead::Vector3f& startPos, f32 distance) {
    const sead::Vector3f& gravity = al::getGravity(actor);
    al::TriangleFilterGroundOnly filter(gravity);
    return alCollisionUtil::getHitPosOnArrow(actor, pos, startPos, gravity * distance, nullptr,
                                             &filter);
}

}  // namespace rs
