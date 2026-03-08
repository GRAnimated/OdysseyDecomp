#include "Npc/HomeCapStateMoveChair.h"

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/ParabolicPath.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/HomeCapFunction.h"
#include "Util/PlayerUtil.h"

namespace {
NERVE_IMPL(HomeCapStateMoveChair, Move);
NERVES_MAKE_NOSTRUCT(HomeCapStateMoveChair, Move);
}  // namespace

HomeCapStateMoveChair::HomeCapStateMoveChair(al::LiveActor* actor)
    : al::ActorStateBase(u8"椅子に移動", actor) {
    mPath = new al::ParabolicPath();
    mOtherChair = nullptr;
}

void HomeCapStateMoveChair::init() {
    initNerve(&Move, 0);
}

void HomeCapStateMoveChair::appear() {
    mIsDead = false;
}

// NON_MATCHING: instruction scheduling — target uses LDP/STP for vector field pairs
void HomeCapStateMoveChair::appearMoveOtherChair(al::LiveActor* otherChair) {
    al::LiveActor* actor = mActor;
    mOtherChair = otherChair;

    const sead::Vector3f& playerPos = rs::getPlayerPos(actor);
    sead::Vector3f pos;
    pos.x = playerPos.x + 0.0f;
    pos.y = playerPos.y + 60.0f;
    pos.z = playerPos.z + 0.0f;
    al::resetPosition(actor, pos);

    sead::Vector3f frontDir = sead::Vector3f::zero;
    sead::Vector3f upDir = sead::Vector3f::zero;
    rs::calcPlayerFrontDir(&frontDir, actor);
    rs::calcPlayerUpDir(&upDir, actor);

    sead::Quatf quat = sead::Quatf::unit;
    al::makeQuatFrontUp(&quat, frontDir, upDir);
    al::setQuat(actor, quat);
    al::setNerve(this, &Move);
}

// NON_MATCHING: instruction scheduling — target uses LDP/STP for vector field pairs
void HomeCapStateMoveChair::getCapAppearPos(sead::Vector3f* outPos) {
    const sead::Vector3f& playerPos = rs::getPlayerPos(mActor);
    outPos->x = playerPos.x + 0.0f;
    outPos->y = playerPos.y + 60.0f;
    outPos->z = playerPos.z + 0.0f;
}

// NON_MATCHING: regalloc differences in inline quaternion rotation (q * Quat(0,v) * conj(q))
void HomeCapStateMoveChair::exeMove() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        const sead::Vector3f& offset = HomeCapFunction::getAimChairOffset();
        sead::Quatf quat = sead::Quatf::unit;
        al::calcQuat(&quat, mOtherChair);

        sead::Quatf vecQuat(0, offset.x, offset.y, offset.z);
        sead::Quatf conjQuat(quat.w, -quat.x, -quat.y, -quat.z);
        sead::Quatf rotated = (quat * vecQuat) * conjQuat;

        const sead::Vector3f& selfTrans = al::getTrans(actor);
        const sead::Vector3f& otherTrans = al::getTrans(mOtherChair);
        sead::Vector3f target;
        target.x = otherTrans.x + rotated.x;
        target.y = otherTrans.y + rotated.y;
        target.z = otherTrans.z + rotated.z;

        mPath->initFromUpVectorAddHeight(selfTrans, target, sead::Vector3f::ey, 50.0f);
        al::startAction(actor, "HomeCapManHeroAppear");
    }

    sead::Vector3f* transPtr = al::getTransPtr(actor);
    f32 rate = al::calcNerveRate(this, 40);
    mPath->calcPosition(transPtr, rate);

    sead::Vector3f dir = sead::Vector3f::zero;
    al::calcFrontDir(&dir, mOtherChair);
    al::turnToDirection(actor, dir, 2.5f);

    if (al::isGreaterEqualStep(this, 40)) {
        sead::Quatf quat = sead::Quatf::unit;
        al::makeQuatFrontUp(&quat, dir, sead::Vector3f::ey);
        al::setQuat(actor, quat);
        kill();
    }
}
