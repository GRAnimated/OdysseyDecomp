#include "Npc/HomeCapStateMoveSouvenir.h"

#include <container/seadPtrArray.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/HomeCapFunction.h"
#include "Npc/HomeCapMovePoint.h"
#include "System/GameDataHolderAccessor.h"
#include "Util/ClothUtil.h"

namespace {
NERVE_IMPL(HomeCapStateMoveSouvenir, TurnNextMovePoint);
NERVE_IMPL(HomeCapStateMoveSouvenir, MoveSouvenirFront);
NERVE_IMPL(HomeCapStateMoveSouvenir, WaitSouvenirAction);
NERVE_IMPL(HomeCapStateMoveSouvenir, TurnSouvenirDir);
NERVE_IMPL(HomeCapStateMoveSouvenir, WaitSouvenirFront);
NERVE_IMPL(HomeCapStateMoveSouvenir, ActionSouvenirFront);
NERVE_IMPL_(HomeCapStateMoveSouvenir, TurnNextMovePointAction, TurnNextMovePoint);
NERVE_IMPL(HomeCapStateMoveSouvenir, ReturnChairTurn);
NERVE_IMPL(HomeCapStateMoveSouvenir, ReturnChair);
NERVE_IMPL(HomeCapStateMoveSouvenir, TurnChairFront);

NERVES_MAKE_NOSTRUCT(HomeCapStateMoveSouvenir, TurnNextMovePoint, MoveSouvenirFront,
                     WaitSouvenirAction, TurnSouvenirDir, WaitSouvenirFront,
                     ActionSouvenirFront, TurnNextMovePointAction, ReturnChairTurn,
                     ReturnChair, TurnChairFront);
}  // namespace

// NON_MATCHING: argument load order and stack layout for GameDataHolderAccessor
static bool tryFindSouvenir(s32* outIndex, al::LiveActor* actor,
                            const sead::PtrArray<HomeCapMovePoint>* movePoints,
                            sead::Buffer<bool>* trackBuffer) {
    s32 count = movePoints->size();
    s32 start = al::getRandom(0, count);

    for (s32 i = start; i < count; i++) {
        GameDataHolderAccessor accessor(
            static_cast<const al::IUseSceneObjHolder*>(actor));
        if (rs::isHaveGift(accessor, movePoints->at(i)->name) && !(*trackBuffer)[i])
            return *outIndex = i, true;
    }

    for (s32 i = 0; i < start; i++) {
        GameDataHolderAccessor accessor(
            static_cast<const al::IUseSceneObjHolder*>(actor));
        if (rs::isHaveGift(accessor, movePoints->at(i)->name) && !(*trackBuffer)[i])
            return *outIndex = i, true;
    }

    return false;
}

HomeCapStateMoveSouvenir::HomeCapStateMoveSouvenir(
    al::LiveActor* actor, const sead::PtrArray<HomeCapMovePoint>& movePoints)
    : al::ActorStateBase(u8"お土産移動", actor), mMovePoints(&movePoints) {}

void HomeCapStateMoveSouvenir::init() {
    s32 count = mMovePoints->size();
    mTrackBuffer.tryAllocBuffer(count, nullptr);
    initNerve(&TurnNextMovePoint, 0);
}

void HomeCapStateMoveSouvenir::appear() {
    mIsDead = false;
    al::setNerve(this, &TurnNextMovePoint);
}

// NON_MATCHING: Buffer operator[] in clearing loop generates unrolled bounds-checking code
bool HomeCapStateMoveSouvenir::tryMoveSouvenir() {
    for (s32 i = 0; i < mTrackBuffer.size(); i++)
        mTrackBuffer[i] = false;

    s32 index = 0;
    if (!tryFindSouvenir(&index, mActor, mMovePoints, &mTrackBuffer))
        return false;

    mMovePointIndex = index;
    return true;
}

void HomeCapStateMoveSouvenir::setReturnChair(al::LiveActor* returnChair) {
    mReturnChair = returnChair;
}

void HomeCapStateMoveSouvenir::exeTurnNextMovePoint() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        al::startAction(actor, "SouvenirTurn");
        al::calcQuat(&mSavedQuat, actor);
    }

    if (al::turnToTarget(actor, mMovePoints->at(mMovePointIndex)->position, 2.0f)) {
        if (al::isNerve(this, &TurnNextMovePoint))
            al::setNerve(this, &MoveSouvenirFront);
        else
            al::setNerve(this, &WaitSouvenirAction);
    }
}

// NON_MATCHING: compiler generates extra cbz branch before cmp/b.eq chain
void HomeCapStateMoveSouvenir::exeWaitSouvenirAction() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        s32 random = al::getRandom(0, 3);
        if (random == 1)
            al::startAction(actor, "SouvenirWait2");
        else if (random != 0)
            al::startAction(actor, "SouvenirWait3");
        else
            al::startAction(actor, "SouvenirWait1");
    }

    if (al::isActionEnd(actor))
        al::setNerve(this, &MoveSouvenirFront);
}

// NON_MATCHING: x21 regalloc for mSavedPos address, reordered sead::Vector3f::zero init
void HomeCapStateMoveSouvenir::exeMoveSouvenirFront() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        al::startAction(actor, "SouvenirMove");
        mSavedPos = al::getTrans(actor);
    }

    f32 rate = al::calcNerveRate(this, 120);
    sead::Vector3f pos = sead::Vector3f::zero;
    al::lerpVec(&pos, mSavedPos, mMovePoints->at(mMovePointIndex)->position, rate);
    al::setTrans(actor, pos);

    if (al::isGreaterEqualStep(this, 120))
        al::setNerve(this, &TurnSouvenirDir);
}

// NON_MATCHING: x22 regalloc for &mSavedQuat address
void HomeCapStateMoveSouvenir::exeTurnSouvenirDir() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        al::startAction(actor, "SouvenirTurn");
        al::calcQuat(&mSavedQuat, actor);
    }

    const sead::Quatf& targetDir = mMovePoints->at(mMovePointIndex)->direction;
    sead::Quatf quat = sead::Quatf::unit;
    f32 rate = al::getNerveStep(this) / 27.0f;
    al::slerpQuat(&quat, mSavedQuat, targetDir, rate);
    al::setQuat(actor, quat);

    if (al::isGreaterEqualStep(this, 27))
        al::setNerve(this, &WaitSouvenirFront);
}

void HomeCapStateMoveSouvenir::exeWaitSouvenirFront() {
    if (al::isGreaterEqualStep(this, 120))
        al::setNerve(this, &ActionSouvenirFront);
}

void HomeCapStateMoveSouvenir::exeActionSouvenirFront() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        al::startAction(actor, "SouvenirWait");
        al::calcQuat(&mSavedQuat, actor);
    }

    if (al::isActionEnd(actor)) {
        mTrackBuffer[mMovePointIndex] = true;

        s32 nextIndex = 0;
        if (tryFindSouvenir(&nextIndex, mActor, mMovePoints, &mTrackBuffer)) {
            mMovePointIndex = nextIndex;
            al::setNerve(this, &TurnNextMovePointAction);
        } else {
            al::setNerve(this, &ReturnChairTurn);
        }
    }
}

void HomeCapStateMoveSouvenir::exeReturnChairTurn() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        al::startAction(actor, "SouvenirTurn");
        al::calcQuat(&mSavedQuat, actor);
    }

    const sead::Vector3f& chairTrans = al::getTrans(mReturnChair);
    const sead::Vector3f& offset = HomeCapFunction::getAimChairOffset();
    sead::Vector3f target = {chairTrans.x + offset.x, chairTrans.y + offset.y,
                             chairTrans.z + offset.z};

    if (al::turnToTarget(actor, target, 2.0f))
        al::setNerve(this, &ReturnChair);
}

void HomeCapStateMoveSouvenir::exeReturnChair() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        al::startAction(actor, "SouvenirMove");
        mSavedPos = al::getTrans(actor);
    }

    f32 rate = al::calcNerveRate(this, 120);
    const sead::Vector3f& chairTrans = al::getTrans(mReturnChair);
    const sead::Vector3f& offset = HomeCapFunction::getAimChairOffset();
    sead::Vector3f target = {chairTrans.x + offset.x, chairTrans.y + offset.y,
                             chairTrans.z + offset.z};

    sead::Vector3f pos = sead::Vector3f::zero;
    al::lerpVec(&pos, mSavedPos, target, rate);
    al::setTrans(actor, pos);

    if (al::isGreaterEqualStep(this, 120))
        al::setNerve(this, &TurnChairFront);
}

void HomeCapStateMoveSouvenir::exeTurnChairFront() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this))
        al::startAction(actor, "SouvenirTurn");

    sead::Vector3f front = sead::Vector3f::zero;
    al::calcFrontDir(&front, mReturnChair);

    if (al::turnToDirection(actor, front, 2.0f))
        kill();
}
