#include "Player/YoshiStateHackWallCling.h"

#include <math/seadMathCalcCommon.h>

#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/Collision/PartsConnector.h"
#include "Library/Collision/PartsConnectorUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"

namespace {
NERVE_IMPL(YoshiStateHackWallCling, Cling)
NERVES_MAKE_NOSTRUCT(YoshiStateHackWallCling, Cling)
}  // namespace

YoshiStateHackWallCling::YoshiStateHackWallCling(al::LiveActor* actor,
                                                 IUsePlayerHack** playerHack,
                                                 const PlayerConst* playerConst,
                                                 const IUsePlayerCollision* collision,
                                                 PlayerAnimator* animator)
    : HackerStateBase("壁接着", actor, playerHack), mPlayerConst(playerConst), mCollision(collision),
      mAnimator(animator) {
    mConnector = al::createCollisionPartsConnector(actor, sead::Quatf::unit);
    initNerve(&Cling, 0);
}

// NON_MATCHING: target/current are both 272 bytes and differ first at the X-component scale multiply operand order (target `FMUL S2, S0, S2`, current `FMUL S2, S2, S0`); next source-level hypothesis is a vector-expression form that preserves the target packed load schedule while forcing scalar-first multiplication.
void YoshiStateHackWallCling::appear() {
    al::LiveActor* actor = mActor;
    HackerStateBase::appear();
    al::setVelocityZero(actor);

    sead::Quatf quat = sead::Quatf::unit;
    {
        sead::Vector3f front = -mNormal;
        sead::Vector3f up = -al::getGravity(actor);
        al::makeQuatFrontUp(&quat, front, up);
    }
    al::updatePoseQuat(actor, quat);
    al::attachCollisionPartsConnector(mConnector, mCollisionParts);

    sead::Vector3f trans =
        mPosition + (mPlayerConst->getCollisionRadius() - 5.0f) * mNormal;
    al::setConnectorBaseQuatTrans(quat, trans, mConnector);
    al::setNerve(this, &Cling);
}

void YoshiStateHackWallCling::setup(const al::CollisionParts* collisionParts,
                                    const sead::Vector3f& position,
                                    const sead::Vector3f& normal) {
    mCollisionParts = collisionParts;
    mPosition = position;
    mNormal = normal;
}

// NON_MATCHING: target/current are both 848 bytes with the same 0x70-byte frame; target assigns the persistent front vector to SP+0x10 and reuses the prior SafeString slot at SP+0x20 for oppositeFront, while current swaps those two vector slots; next source-level hypothesis is a source lifetime/declaration shape that makes Clang reuse the SafeString slot for oppositeFront without extending either vector lifetime.
void YoshiStateHackWallCling::exeCling() {
    if (!al::isMtxConnectorConnecting(mConnector))
        kill();

    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        al::startHitReaction(actor, "壁吸着");
        if (rs::isPlayerSideFaceToCameraZ(actor))
            mAnimator->startAnim("WallKeep");
        else
            mAnimator->startAnim("WallKeepReverse");
    }

    al::connectPoseQT(actor, mConnector);

    sead::Vector3f front{0.0f, 0.0f, 0.0f};
    al::calcFrontDir(&front, actor);
    al::setVelocity(actor, front * 5.0f);

    {
        const IUsePlayerCollision* collision = mCollision;
        sead::Vector3f oppositeFront{0.0f, 0.0f, 0.0f};
        al::calcFrontDir(&oppositeFront, actor);
        oppositeFront.negate();
        al::normalize(&oppositeFront);

        if (rs::isCollidedGround(collision) &&
            oppositeFront.dot(rs::getCollidedGroundNormal(collision)) < 0.98481f) {
            kill();
            return;
        }
        if (rs::isCollidedWall(collision) &&
            oppositeFront.dot(rs::getCollidedWallNormal(collision)) < 0.86603f) {
            kill();
            return;
        }
        if (rs::isCollidedCeiling(collision) &&
            oppositeFront.dot(rs::getCollidedCeilingNormal(collision)) < 0.98481f) {
            kill();
            return;
        }
    }

    if (rs::isCollidedWall(mCollision) &&
        rs::getCollidedWallCollisionParts(mCollision) != mCollisionParts) {
        const sead::Vector3f& wallPos = rs::getCollidedWallPos(mCollision);
        const sead::Vector3f& trans = al::getTrans(mActor);
        const f32 distance = sead::Mathf::max(0.0f, (wallPos - trans).dot(front));
        const f32 length =
            sead::Mathf::max(0.0f, mPlayerConst->getCollisionRadius() - distance) + 5.0f;
        const sead::Vector3f arrow = front * length;
        const al::ArrowHitInfo* hitInfo = nullptr;
        if (alCollisionUtil::getLastPolyOnArrow(mActor, &hitInfo, wallPos, arrow, nullptr, nullptr) &&
            alCollisionUtil::getCollisionHitParts(hitInfo->hitInfo.data()) == mCollisionParts) {
            kill();
            return;
        }
    }

    if (rs::isCollidedDamageCodeAnyWallHit(mCollision))
        kill();
}

YoshiStateHackWallCling::~YoshiStateHackWallCling() = default;
