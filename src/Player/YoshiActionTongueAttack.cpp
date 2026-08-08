#include "Player/YoshiActionTongueAttack.h"

#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"

#include "Player/PlayerAnimator.h"
#include "Player/YoshiTongue.h"
#include "Util/PlayerHackInputFunction.h"

YoshiActionTongueAttack::YoshiActionTongueAttack(const al::LiveActor* player,
                                                 IUsePlayerHack** playerHack,
                                                 PlayerAnimator* animator, YoshiTongue* tongue)
    : mPlayer(player), mPlayerHack(playerHack), mAnimator(animator), mTongue(tongue) {}

bool YoshiActionTongueAttack::tryStartAttack(const sead::Vector3f& up) {
    IUsePlayerHack* playerHack = *mPlayerHack;
    YoshiTongue* tongue = mTongue;
    if ((!rs::isTriggerHackAction(playerHack) && !rs::isTriggerHackSwing(playerHack)) ||
        !tongue->isEnableStartAttack())
        return false;

    sead::Vector3f attackDir{0.0f, 0.0f, 0.0f};
    const al::LiveActor* player = mPlayer;
    if (!rs::calcHackerMoveDir(&attackDir, playerHack, up)) {
        sead::Vector3f playerUp{0.0f, 0.0f, 0.0f};
        al::calcUpDir(&playerUp, player);
        al::calcFrontDir(&attackDir, player);
        al::alongVectorNormalH(&attackDir, attackDir, playerUp, up);
        al::normalize(&attackDir);
    }

    mIsUseSubParts = false;
    startTongueAnimStart();
    mTongue->startAttack(attackDir, up);
    return true;
}

void YoshiActionTongueAttack::initStartTongue(bool useSubParts) {
    mIsUseSubParts = useSubParts;
}

void YoshiActionTongueAttack::startTongueAnimStart() {
    if (mIsUseSubParts)
        mAnimator->startUpperBodyAnimSubParts("EatStart");
    else
        mAnimator->startUpperBodyAnim("EatStart");
}

bool YoshiActionTongueAttack::tryStartAttackGround(const sead::Vector3f& groundUp) {
    IUsePlayerHack* playerHack = *mPlayerHack;
    YoshiTongue* tongue = mTongue;
    if ((!rs::isTriggerHackAction(playerHack) && !rs::isTriggerHackSwing(playerHack)) ||
        !tongue->isEnableStartAttack())
        return false;

    sead::Vector3f attackDir{0.0f, 0.0f, 0.0f};
    const al::LiveActor* player = mPlayer;
    if (!rs::calcHackerMoveDir(&attackDir, playerHack, groundUp)) {
        sead::Vector3f playerUp{0.0f, 0.0f, 0.0f};
        al::calcUpDir(&playerUp, player);
        al::calcFrontDir(&attackDir, player);
        al::alongVectorNormalH(&attackDir, attackDir, playerUp, groundUp);
        al::normalize(&attackDir);
    }

    sead::Vector3f up = -al::getGravity(mPlayer);
    if (attackDir.dot(up) > 0.0f) {
        const al::LiveActor* movePlayer = mPlayer;
        if (!rs::calcHackerMoveDir(&attackDir, playerHack, up)) {
            sead::Vector3f playerUp{0.0f, 0.0f, 0.0f};
            al::calcUpDir(&playerUp, movePlayer);
            al::calcFrontDir(&attackDir, movePlayer);
            al::alongVectorNormalH(&attackDir, attackDir, playerUp, up);
            al::normalize(&attackDir);
        }
    }

    mIsUseSubParts = false;
    startTongueAnimStart();
    mTongue->startAttack(attackDir, groundUp);
    return true;
}

bool YoshiActionTongueAttack::tryStartAttackFromWall(const sead::Vector3f& up,
                                                     const sead::Vector3f& wallDir) {
    IUsePlayerHack* playerHack = *mPlayerHack;
    YoshiTongue* tongue = mTongue;
    if (!rs::isTriggerHackAction(playerHack) && !rs::isTriggerHackSwing(playerHack))
        return false;
    if (!tongue->isEnableStartAttack())
        return false;

    sead::Vector3f attackDir{0.0f, 0.0f, 0.0f};
    const al::LiveActor* player = mPlayer;
    if (!rs::calcHackerMoveDir(&attackDir, playerHack, up)) {
        sead::Vector3f playerUp{0.0f, 0.0f, 0.0f};
        al::calcUpDir(&playerUp, player);
        al::calcFrontDir(&attackDir, player);
        al::alongVectorNormalH(&attackDir, attackDir, playerUp, up);
        al::normalize(&attackDir);
        attackDir.set(wallDir);
    }
    if (attackDir.dot(wallDir) < -0.5f)
        return false;
    al::limitVectorOppositeDir(&attackDir, wallDir, attackDir, attackDir.length());
    al::normalize(&attackDir);
    mIsUseSubParts = true;
    startTongueAnimStart();
    mTongue->startAttack(attackDir, up);
    return true;
}

bool YoshiActionTongueAttack::tryStartAttackFromWallAir(const sead::Vector3f& up,
                                                        const sead::Vector3f& wallDir) {
    IUsePlayerHack* playerHack = *mPlayerHack;
    YoshiTongue* tongue = mTongue;
    if ((!rs::isTriggerHackAction(playerHack) && !rs::isTriggerHackSwing(playerHack)) ||
        !tongue->isEnableStartAttack())
        return false;

    sead::Vector3f attackDir{0.0f, 0.0f, 0.0f};
    const al::LiveActor* player = mPlayer;
    if (!rs::calcHackerMoveDir(&attackDir, playerHack, up)) {
        sead::Vector3f playerUp{0.0f, 0.0f, 0.0f};
        al::calcUpDir(&playerUp, player);
        al::calcFrontDir(&attackDir, player);
        al::alongVectorNormalH(&attackDir, attackDir, playerUp, up);
        al::normalize(&attackDir);
        attackDir.set(wallDir);
    }
    if (attackDir.dot(wallDir) < -0.96593f) {
        attackDir.set(wallDir);
        attackDir.negate();
    } else {
        al::limitVectorOppositeDir(&attackDir, wallDir, attackDir, attackDir.length());
    }
    al::normalize(&attackDir);
    mIsUseSubParts = true;
    startTongueAnimStart();
    mTongue->startAttack(attackDir, up);
    return true;
}

bool YoshiActionTongueAttack::tryStartEatFinish() {
    if (!mTongue->isEnableEatFinish())
        return false;

    mTongue->eatFinish();
    if (mIsUseSubParts)
        mAnimator->startUpperBodyAnimSubParts("EatEnd");
    else
        mAnimator->startUpperBodyAnim("EatEnd");
    return true;
}

void YoshiActionTongueAttack::startTongueAnimEnd() {
    if (mIsUseSubParts)
        mAnimator->startUpperBodyAnimSubParts("EatEnd");
    else
        mAnimator->startUpperBodyAnim("EatEnd");
}

bool YoshiActionTongueAttack::tryEatDirect() {
    startTongueAnimEnd();
    return true;
}

void YoshiActionTongueAttack::startShrink() {
    mTongue->startShrink();
}

void YoshiActionTongueAttack::endShrink() {
    if (mTongue->isShrinkMove())
        mTongue->endShrink();
}

void YoshiActionTongueAttack::adjustTongueShrinkRestRange(f32 range) {
    mTongue->adjustShrinkRestRange(range);
}

void YoshiActionTongueAttack::update() {
    mIsUpperBodyAnimAttached = false;
    if (!mAnimator->isUpperBodyAnimAttached())
        return;

    mIsUpperBodyAnimAttached = true;
    if (mAnimator->isUpperBodyAnim("EatStart")) {
        if (!mAnimator->isUpperBodyAnimEnd())
            return;
        startTongueAnimLoop();
        return;
    }

    if (mAnimator->isUpperBodyAnim("EatEnd")) {
        if (!mAnimator->isUpperBodyAnimEnd())
            return;
        if (mTongue->isExistEatBind()) {
            startTongueAnimLoop();
            return;
        }
    } else if (!mTongue->isEnableStartAttack()) {
        return;
    }
    endTongueAnim();
}

void YoshiActionTongueAttack::startTongueAnimLoop() {
    if (mIsUseSubParts)
        mAnimator->startUpperBodyAnimSubParts("Eat");
    else
        mAnimator->startUpperBodyAnim("Eat");
}

void YoshiActionTongueAttack::endTongueAnim() {
    if (!mAnimator->isUpperBodyAnimAttached())
        return;

    mAnimator->clearUpperBodyAnim();
    mIsUpperBodyAnimAttached = false;
    if (!mIsUseSubParts)
        mAnimator->restartYoshiActionVis();
}

void YoshiActionTongueAttack::syncEatVis() {
    if (!mIsUseSubParts && mIsUpperBodyAnimAttached)
        mAnimator->overwrideYoshiEatVis();
}
