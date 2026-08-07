#include "Player/PlayerStatePress.h"

#include "Library/Action/ActorActionKeeper.h"
#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerAnimator.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
NERVE_IMPL(PlayerStatePress, Press)
NERVES_MAKE_STRUCT(PlayerStatePress, Press)
}  // namespace

PlayerStatePress::PlayerStatePress(al::LiveActor* player, const IUsePlayerCollision* collision,
                                   PlayerAnimator* animator)
    : al::ActorStateBase("潰れ", player), mCollision(collision), mAnimator(animator),
      mOriginalScale{1.0f, 1.0f, 1.0f}, mScale{1.0f, 1.0f, 1.0f},
      mOriginalTrans(sead::Vector3f::zero), mTrans(sead::Vector3f::zero) {
    initNerve(&NrvPlayerStatePress.Press, 0);
}

PlayerStatePress::~PlayerStatePress() = default;

// NON_MATCHING: behavior follows the corpus, but the large pose/collision setup has not yet been
// checked for exact local ordering; next compare the first divergent block after the initial setup.
void PlayerStatePress::appear() {
    al::ActorStateBase::appear();

    al::LiveActor* actor = mActor;
    const sead::Vector3f& gravity = al::getGravity(actor);
    if (mAnimator) {
        mAnimator->startPress();
    } else {
        if (al::isSklAnimPlaying(actor, 0))
            al::setSklAnimBlendFrameRateAll(actor, 0.0f, true);
        actor->getActorActionKeeper()->tryStartActionNoAnim("Press");
    }

    al::offCollide(actor);
    al::setVelocityZero(actor);
    al::startHitReaction(actor, "圧死");

    mOriginalScale = al::getScale(actor);
    mScale = mOriginalScale;
    mOriginalTrans = al::getTrans(actor);
    mTrans = mOriginalTrans;

    const sead::Vector3f& pressNormal = rs::getPressCollisionNormal(mCollision);
    sead::Vector3f hitPos = al::getTrans(actor);
    sead::Vector3f pressDir = -pressNormal;
    rs::calcGroundNormalOrUpDir(&pressDir, actor, mCollision);

    sead::Vector3f arrowStart = al::getTrans(actor) - pressNormal * 20.0f;
    sead::Vector3f arrow = pressNormal * 220.0f;
    alCollisionUtil::getHitPosAndNormalOnArrow(actor, &hitPos, &pressDir, arrowStart, arrow,
                                               nullptr, nullptr);

    if (rs::isPressedGround(mCollision)) {
        sead::Vector3f groundNormal = sead::Vector3f::zero;
        rs::calcGroundNormalOrUpDir(&groundNormal, actor, mCollision);
        rs::slerpUp(actor, groundNormal, 1.0f, 180.0f);
        mScale.y = 0.1f;

        sead::Vector3f offset = sead::Vector3f::zero;
        if (pressDir.dot(gravity) > 0.0f)
            offset = pressDir * 15.0f;
        mTrans = mOriginalTrans + (hitPos + offset - mOriginalTrans);
    } else if (rs::isPressedWall(mCollision)) {
        sead::Vector3f front = sead::Vector3f::zero;
        al::calcFrontDir(&front, actor);

        sead::Vector3f wallUp = sead::Vector3f::zero;
        al::verticalizeVec(&wallUp, gravity, pressDir);
        al::tryNormalizeOrZero(&wallUp);

        sead::Vector3f parallel = sead::Vector3f::zero;
        sead::Vector3f vertical = sead::Vector3f::zero;
        al::separateVectorParallelVertical(&parallel, &vertical, wallUp, front);

        sead::Quatf quat = sead::Quatf::unit;
        sead::Vector3f poseDir;
        if (parallel.length() >= vertical.length()) {
            poseDir = front;
            f32 dot = poseDir.dot(wallUp);
            if (!al::isNearZero(dot, 0.001f))
                poseDir = dot > 0.0f ? wallUp : -wallUp;
            al::makeQuatFrontUp(&quat, poseDir, -gravity);
        } else {
            al::calcSideDir(&poseDir, actor);
            f32 dot = poseDir.dot(wallUp);
            if (!al::isNearZero(dot, 0.001f))
                poseDir = dot > 0.0f ? wallUp : -wallUp;
            al::makeQuatSideUp(&quat, poseDir, -gravity);
        }
        al::updatePoseQuat(actor, quat);

        al::calcFrontDir(&front, actor);
        sead::Vector3f wallNormal = sead::Vector3f::zero;
        al::verticalizeVec(&wallNormal, gravity, pressNormal);
        al::tryNormalizeOrZero(&wallNormal);
        parallel = sead::Vector3f::zero;
        vertical = sead::Vector3f::zero;
        al::separateVectorParallelVertical(&parallel, &vertical, wallNormal, front);
        if (parallel.length() >= vertical.length())
            mScale.z = 0.1f;
        else
            mScale.x = 0.1f;

        mTrans = mOriginalTrans + (hitPos + pressDir * 5.0f - mOriginalTrans);
    }

    al::setNerve(this, &NrvPlayerStatePress.Press);
}

void PlayerStatePress::exePress() {
    f32 rate = al::calcNerveRate(this, 10);

    sead::Vector3f scale = {1.0f, 1.0f, 1.0f};
    al::lerpVec(&scale, mOriginalScale, mScale, rate);
    al::setScale(mActor, scale);

    sead::Vector3f trans = {0.0f, 0.0f, 0.0f};
    al::lerpVec(&trans, mOriginalTrans, mTrans, rate);
    al::setTrans(mActor, trans);

    if (!al::isLessEqualStep(this, 60))
        kill();
}
