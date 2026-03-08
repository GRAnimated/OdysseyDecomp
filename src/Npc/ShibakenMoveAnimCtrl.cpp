#include "Npc/ShibakenMoveAnimCtrl.h"

#include "Library/Effect/EffectSystemInfo.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
NERVE_IMPL(ShibakenMoveAnimCtrl, Move);
NERVE_IMPL(ShibakenMoveAnimCtrl, WalkSniffStart);
NERVE_IMPL(ShibakenMoveAnimCtrl, WalkSniff);
NERVE_IMPL(ShibakenMoveAnimCtrl, WalkSniffEnd);
NERVES_MAKE_NOSTRUCT(ShibakenMoveAnimCtrl, Move, WalkSniffStart, WalkSniff, WalkSniffEnd);
}  // namespace

ShibakenMoveAnimCtrl::ShibakenMoveAnimCtrl(al::LiveActor* actor, const f32& runSpeed,
                                           const f32& walkSpeed, const f32& sniffSpeed)
    : al::NerveExecutor("忠犬の移動アニメ制御") {
    mActor = actor;
    mWalkRunRatio = al::getSklAnimFrameMax(actor, "Walk") / al::getSklAnimFrameMax(actor, "Run");
    mWalkSniffRatio =
        al::getSklAnimFrameMax(actor, "Walk") / al::getSklAnimFrameMax(actor, "WalkSniff");
    mWalkSniffWalkRatio =
        al::getSklAnimFrameMax(actor, "Walk") / al::getSklAnimFrameMax(actor, "WalkSniffWalk");
    mIsSniffing = false;
    mRunSpeed = &runSpeed;
    mWalkSpeed = &walkSpeed;
    mMaxSpeed = &sniffSpeed;
    initNerve(&Move, 0);
}

void ShibakenMoveAnimCtrl::update() {
    updateNerve();

    bool isRunning = false;

    if (al::isActionPlaying(mActor, "Move")) {
        f32 walkBlend;

        if (al::getSklAnimBlendWeight(mActor, 3) < 0.01f) {
            walkBlend = 1.0f;
        } else {
            f32 w0 = al::getSklAnimBlendWeight(mActor, 0);
            f32 w0b = al::getSklAnimBlendWeight(mActor, 0);
            f32 w3 = al::getSklAnimBlendWeight(mActor, 3);
            walkBlend = al::normalize(w0, 0.0f, w0b + w3);
        }

        f32 walkWeight;
        f32 sniffWeight;

        if (al::isNerve(this, &WalkSniffStart)) {
            sniffWeight = al::calcNerveEaseInOutRate(this, 16);
            walkWeight = 1.0f - sniffWeight;
        } else if (al::isNerve(this, &WalkSniff)) {
            walkWeight = 0.0f;
            sniffWeight = 1.0f;
        } else if (al::isNerve(this, &WalkSniffEnd)) {
            walkWeight = al::calcNerveEaseInOutRate(this, 16);
            sniffWeight = 1.0f - walkWeight;
        } else {
            walkWeight = 1.0f;
            sniffWeight = 0.0f;
        }

        al::LiveActor* actor = mActor;
        sead::Vector3f frontDir = {0.0f, 0.0f, 0.0f};
        al::calcFrontDir(&frontDir, actor);
        const sead::Vector3f& velocity = al::getVelocity(actor);
        f32 currentSpeed =
            frontDir.x * velocity.x + frontDir.y * velocity.y + frontDir.z * velocity.z;

        f32 speedMaxWalk = al::calcSpeedMax(*mRunSpeed, *mMaxSpeed);

        f32 blendA = 0.0f;
        f32 blendB = 0.0f;

        if (walkWeight > 0.0f) {
            f32 runBlend = 1.0f;

            if (speedMaxWalk < currentSpeed) {
                f32 speedMaxRun = al::calcSpeedMax(*mWalkSpeed, *mMaxSpeed);
                runBlend = al::squareIn(1.0f - al::normalize(currentSpeed, speedMaxWalk,
                                                             speedMaxRun));
            }

            f32 blendW0 = al::getSklAnimBlendWeight(mActor, 0);
            f32 blendW3 = al::getSklAnimBlendWeight(mActor, 3);
            blendA = al::lerpValue(blendW0 + blendW3, runBlend, 0.15f);
            blendB = 1.0f - blendA;
        }

        f32 sniffBlendA = walkWeight * blendA;
        f32 walkBlendB = walkWeight * blendB;
        f32 sniffTarget = al::lerpValue(1.0f - walkBlend, mIsSniffing ? 1.0f : 0.0f, 0.15f);
        f32 sniffWalkBlend = sniffBlendA * sniffTarget;
        f32 sniffBlend = sniffBlendA - sniffWalkBlend;

        al::setSklAnimBlendWeightQuad(mActor, sniffBlend, walkBlendB, sniffWeight, sniffWalkBlend);

        f32 frameRateScale = 1.0f;
        if (currentSpeed < speedMaxWalk)
            frameRateScale = al::lerpValue(0.25f, 1.0f, al::normalize(currentSpeed, 0.0f,
                                                                       speedMaxWalk));

        al::LiveActor* actor2 = mActor;
        f32 combinedRate = sniffBlend + walkBlendB * mWalkRunRatio +
                           sniffWeight * mWalkSniffRatio +
                           sniffWalkBlend * mWalkSniffWalkRatio;
        f32 finalRate = al::lerpValue(frameRateScale, 1.0f, sniffWeight) * combinedRate;
        al::setSklAnimBlendFrameRateAll(actor2, finalRate, true);

        isRunning = walkBlendB > 0.8f;
    }

    if (isRunning) {
        if (!al::isEffectEmitting(mActor, "Run"))
            al::emitEffect(mActor, "Run", nullptr);
    } else {
        if (al::isEffectEmitting(mActor, "Run"))
            al::deleteEffect(mActor, "Run");
    }
}

void ShibakenMoveAnimCtrl::startWalkSniff() {
    al::tryStartActionIfNotPlaying(mActor, "Move");
    al::setNerve(this, &WalkSniffStart);
}

void ShibakenMoveAnimCtrl::endWalkSniff() {
    if (al::isNerve(this, &WalkSniffStart) || al::isNerve(this, &WalkSniff))
        al::setNerve(this, &WalkSniffEnd);
}

void ShibakenMoveAnimCtrl::exeMove() {}

void ShibakenMoveAnimCtrl::exeWalkSniffStart() {
    al::setNerveAtGreaterEqualStep(this, &WalkSniff, 16);
}

void ShibakenMoveAnimCtrl::exeWalkSniff() {}

void ShibakenMoveAnimCtrl::exeWalkSniffEnd() {
    al::setNerveAtGreaterEqualStep(this, &Move, 16);
}
