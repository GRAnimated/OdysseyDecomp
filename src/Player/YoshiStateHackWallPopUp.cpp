#include "Player/YoshiStateHackWallPopUp.h"

#include "Library/Collision/PartsConnector.h"
#include "Library/Collision/PartsConnectorUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MatrixUtil.h"
#include "Library/Math/ParabolicPath.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
NERVE_IMPL(YoshiStateHackWallPopUp, PopUp)
NERVES_MAKE_NOSTRUCT(YoshiStateHackWallPopUp, PopUp)
}  // namespace

YoshiStateHackWallPopUp::YoshiStateHackWallPopUp(al::LiveActor* actor,
                                                 IUsePlayerHack** playerHack,
                                                 const PlayerConst* playerConst,
                                                 const IUsePlayerCollision* collision,
                                                 PlayerAnimator* animator)
    : HackerStateBase("壁はね上がり", actor, playerHack), mPlayerConst(playerConst),
      mCollision(collision), mAnimator(animator), mStartTrans(0.0f, 0.0f, 0.0f),
      mStartQuat(sead::Quatf::unit), mCollisionParts(nullptr), mConnector(nullptr),
      mSnapMtx(sead::Matrix34f::ident), mConnectedMtx(sead::Matrix34f::ident), mPath(nullptr) {
    mConnector = al::createCollisionPartsConnector(actor, sead::Quatf::unit);
    mPath = new al::ParabolicPath();
    initNerve(&PopUp, 0);
}

void YoshiStateHackWallPopUp::appear() {
    al::LiveActor* actor = mActor;
    HackerStateBase::appear();
    mStartTrans = al::getTrans(actor);
    al::calcQuat(&mStartQuat, actor);
    al::setVelocityZero(actor);
    al::setNerve(this, &PopUp);
}

void YoshiStateHackWallPopUp::kill() {
    mCollisionParts = nullptr;
    al::disconnectMtxConnector(mConnector);
    HackerStateBase::kill();
}

void YoshiStateHackWallPopUp::setupSnap(const al::CollisionParts* collisionParts,
                                        const sead::Vector3f& position,
                                        const sead::Vector3f& front,
                                        const sead::Vector3f& up) {
    al::startHitReactionHitEffect(mActor, "壁接触ポップアップ", position);
    mCollisionParts = const_cast<al::CollisionParts*>(collisionParts);
    al::makeMtxUpFrontPos(&mSnapMtx, up, front, position);
    mConnectedMtx = mSnapMtx;
    al::attachMtxConnectorToCollisionParts(mConnector, collisionParts);
}

// NON_MATCHING: behavior is recovered; current is 0x354 vs target 0x35c. The target copies the
// connected-matrix translation with three scalar stores/bit loads while current folds X/Y into STP
// and loads Z as FP, accounting for the two-instruction size gap. Next source-level hypothesis is the
// original matrix-translation accessor/source lifetime that preserves scalar copies.
void YoshiStateHackWallPopUp::exePopUp() {
    al::LiveActor* actor = mActor;
    const sead::Vector3f up = -al::getGravity(actor);
    al::calcConnectMtx(&mConnectedMtx, mConnector, mSnapMtx);

    if (al::isFirstStep(this)) {
        mAnimator->startAnim("JumpRollingPopUp");
        mAnimator->setAnimRate(1.5f);
    }

    {
        const sead::Vector3f endTrans = mConnectedMtx.getTranslation();
        const f32 height = sead::Mathf::max(
            100.0f - sead::Mathf::abs((endTrans - mStartTrans).dot(up)), 100.0f);
        mPath->initFromUpVectorAddHeight(mStartTrans, endTrans, up, height);
    }

    const s32 pathTime = mPath->calcPathTimeFromAverageSpeed(30.0f);
    const s32 pathFrames = pathTime < 20 ? 20 : pathTime;
    const s32 endStep = sead::Mathf::ceil(pathFrames);
    const f32 rate = al::calcNerveRate(this, endStep);

    sead::Vector3f pathPos;
    pathPos.set(0.0f, 0.0f, 0.0f);
    mPath->calcPosition(&pathPos, rate);
    sead::Vector3f velocity = pathPos - al::getTrans(actor);
    if (al::isNearZero(velocity, 0.001f) && rs::isCollidedGround(mCollision)) {
        sead::Vector3f groundNormal;
        groundNormal.set(0.0f, 0.0f, 0.0f);
        rs::calcGroundNormalOrGravityDir(&groundNormal, actor, mCollision);
        velocity -= groundNormal * mPlayerConst->getGravityAir();
    }
    al::setVelocity(actor, velocity);

    sead::Quatf targetQuat = sead::Quatf::unit;
    mConnectedMtx.toQuat(targetQuat);
    sead::Quatf poseQuat = sead::Quatf::unit;
    al::slerpQuat(&poseQuat, mStartQuat, targetQuat, al::easeOut(rate));
    poseQuat.normalize();
    al::updatePoseQuat(actor, poseQuat);

    if (rs::isCollidedCeiling(mCollision)) {
        rs::reflectCeiling(actor, 0.0f);
        kill();
    } else if (al::isGreaterEqualStep(this, endStep)) {
        kill();
    }
}
