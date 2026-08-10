#include "Player/YoshiJudgeWallCling.h"

#include "Library/Collision/CollisionParts.h"
#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/Collision/PartsInterpolator.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"

#include "Player/PlayerConst.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackFunction.h"

YoshiJudgeWallCling::YoshiJudgeWallCling(IUsePlayerHack** hacker, const al::LiveActor* player,
                                         const IUsePlayerCollision* collision,
                                         const PlayerWallActionHistory* wallActionHistory,
                                         const PlayerConst* playerConst)
    : HackerJudge(hacker), mPlayer(player), mCollision(collision),
      mWallActionHistory(wallActionHistory), mConst(playerConst), mIsDamageWall(false),
      mIsJudge(false), mIsWallPopUp(false), mCollisionParts(nullptr),
      mWallPos{0.0f, 0.0f, 0.0f}, mWallNormal{0.0f, 0.0f, 0.0f},
      mPopUpDir{0.0f, 0.0f, 0.0f} {}

// NON_MATCHING: target 1112/current 1060 with 23/23 semantic calls and target 0x110
// frame recovered; target keeps checked height in S9 and preserves collided-wall normal via
// W23/W22/W24 before FMOV to S10-S12. Next hypothesis: recover the original scalar/vector
// lifetime shape that induces those register choices without manual component reconstruction.
void YoshiJudgeWallCling::update() {
    f32 heightSpace;
    {
        IUsePlayerHack** hackerPtr = getHacker();
        const al::LiveActor* player = mPlayer;
        const IUsePlayerCollision* collision = mCollision;
        const PlayerWallActionHistory* wallActionHistory = mWallActionHistory;

        mIsWallPopUp = false;
        mIsDamageWall = false;
        mIsJudge = false;
        IUsePlayerHack* hacker = *hackerPtr;

        if (rs::isCollidedGround(collision) || !rs::isCollidedWall(collision))
            return;

        if (rs::isCollisionCodeDamageWall(collision)) {
            mIsDamageWall = true;
            return;
        }

        if (al::calcSpeedV(player) >= 0.0f && !rs::isCollidedCeiling(collision))
            return;

        const sead::Vector3f& wallNormal = rs::getCollidedWallNormal(collision);
        const sead::Vector3f& gravity = al::getGravity(player);

        if (hacker) {
            heightSpace = 0.0f;
            rs::checkExistHeightSpaceAboveGround(&heightSpace, hacker, 100.0f);
        } else {
            heightSpace = 0.0f;
        }

        sead::Vector3f historyPos = al::getTrans(player) + gravity * heightSpace;
        if (!rs::judgeEnableWallKeepHistory(player, wallActionHistory, historyPos, wallNormal, 0.0f,
                                            false))
            return;
    }

    mIsJudge = true;

    const al::CollisionParts* collidedParts = rs::getCollidedWallCollisionParts(mCollision);
    sead::Vector3f collidedPos;
    collidedPos.set(rs::getCollidedWallPos(mCollision));
    sead::Vector3f collidedNormal;
    collidedNormal.set(rs::getCollidedWallNormal(mCollision));

    sead::Vector3f forceMove(0.0f, 0.0f, 0.0f);
    collidedParts->calcForceMovePower(&forceMove, collidedPos);

    sead::Vector3f arrowStart = al::getTrans(mPlayer) + forceMove + collidedNormal * 10.0f +
                                al::getGravity(mPlayer) * heightSpace;
    f32 wallDepth = sead::Mathf::clampMin(
        (collidedPos - al::getTrans(mPlayer)).dot(-collidedNormal), 0.0f);
    f32 arrowLength = heightSpace + wallDepth + 110.0f;
    sead::Vector3f arrowDir = -collidedNormal * arrowLength;

    const al::ArrowHitInfo* hitInfo = nullptr;
    al::TriangleFilterWallOnly filterWallOnly(al::getGravity(mPlayer));
    if (alCollisionUtil::getFirstPolyOnArrow(mPlayer, &hitInfo, arrowStart, arrowDir, nullptr,
                                             &filterWallOnly)) {
        mCollisionParts = alCollisionUtil::getCollisionHitParts(hitInfo->hitInfo.data());
        mWallNormal.set(alCollisionUtil::getCollisionHitNormal(hitInfo->hitInfo.data()));
        mWallPos.set(alCollisionUtil::getCollisionHitPos(hitInfo->hitInfo.data()));
    } else {
        mCollisionParts = collidedParts;
        mWallPos.set(collidedPos);
        mWallNormal.set(collidedNormal);
    }

    const sead::Vector3f reverseWallNormal = -mWallNormal;
    const al::CollisionParts* popUpCollisionParts = nullptr;
    sead::Vector3f popUpWallPos(0.0f, 0.0f, 0.0f);
    sead::Vector3f popUpDir(0.0f, 0.0f, 0.0f);
    mIsWallPopUp = rs::findYoshiWallPopUpPos(
        &popUpCollisionParts, &popUpWallPos, &popUpDir, mPlayer, reverseWallNormal, mWallPos,
        mWallNormal, mConst->getWallKeepDegree(), 60.0f, 160.0f, 0.0f, 1.0f,
        mConst->getCollisionRadius(), mConst->getCollisionRadiusStand());
    if (mIsWallPopUp) {
        mCollisionParts = popUpCollisionParts;
        mWallPos.set(popUpWallPos);
        mPopUpDir.set(popUpDir);
    }
}

void YoshiJudgeWallCling::reset() {
    mIsDamageWall = false;
}

bool YoshiJudgeWallCling::judge() const {
    return mIsJudge;
}
