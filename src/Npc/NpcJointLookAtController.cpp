#include "Npc/NpcJointLookAtController.h"

#include "Library/Joint/JointLookAtController.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"

#include "Npc/TalkNpcParam.h"
#include "Player/PlayerEyeSensorHitHolder.h"
#include "Util/NpcEventFlowUtil.h"

NpcJointLookAtController::NpcJointLookAtController(f32 lookAtDistance)
    : mLookAtDistance(lookAtDistance) {}

NpcJointLookAtController* NpcJointLookAtController::create(al::LiveActor* actor,
                                                           const TalkNpcParam* param) {
    if (param->mJointLookAtCount < 1)
        return nullptr;
    // NON_MATCHING: inlined constructor store coalescing differs (STP XZR,X20 + STR X19 vs STP
    // X20,XZR + STR X19)
    auto* controller = new NpcJointLookAtController(2000.0f);
    controller->mParam = param;
    controller->mActor = actor;
    controller->mJointLookAtController = param->createAndAppendJointLookAtController(actor);
    return controller;
}

NpcJointLookAtController* NpcJointLookAtController::tryCreate(al::LiveActor* actor,
                                                              const TalkNpcParam* param) {
    if (param->mJointLookAtCount < 1)
        return nullptr;
    // NON_MATCHING: same inlined constructor store coalescing issue as create()
    auto* controller = new NpcJointLookAtController(2000.0f);
    controller->mParam = param;
    controller->mActor = actor;
    controller->mJointLookAtController = param->createAndAppendJointLookAtController(actor);
    return controller;
}

NpcJointLookAtController* NpcJointLookAtController::tryCreate(al::LiveActor* actor,
                                                              const TalkNpcParam* param,
                                                              f32 lookAtDistance) {
    if (param->mJointLookAtCount < 1)
        return nullptr;
    auto* controller = new NpcJointLookAtController(lookAtDistance);
    controller->mParam = param;
    controller->mActor = actor;
    controller->mJointLookAtController = param->createAndAppendJointLookAtController(actor);
    return controller;
}

void NpcJointLookAtController::init(al::LiveActor* actor, const TalkNpcParam* param) {
    mActor = actor;
    mParam = param;
    mJointLookAtController = param->createAndAppendJointLookAtController(actor);
}

void NpcJointLookAtController::cancelUpdateRequest() {
    if (mJointLookAtController)
        *reinterpret_cast<u16*>(reinterpret_cast<u8*>(mJointLookAtController) + 0x50) = 0;
}

// NON_MATCHING: sensor pointer caching (X20 vs reload from holder), stack layout differences,
// B.LS vs B.LE after FCMP
void NpcJointLookAtController::update() {
    if (!mJointLookAtController)
        return;

    if (mIsInvalid || rs::isExecuteSceneEvent(mActor)) {
        u8* ctrl = reinterpret_cast<u8*>(mJointLookAtController);
        if (ctrl[0x53]) {
            ctrl[0x53] = 0;
            mLastAnimName = nullptr;
        }
    } else {
        const char* animName = al::getPlayingSklAnimName(mActor, 0);
        if (animName != mLastAnimName) {
            if (animName) {
                if (mParam->isInvalidJointLookSklAnim(animName))
                    reinterpret_cast<u8*>(mJointLookAtController)[0x53] = 0;
                else
                    reinterpret_cast<u8*>(mJointLookAtController)[0x53] = 1;
            }
            mLastAnimName = animName;
        }
    }

    u8* ctrl = reinterpret_cast<u8*>(mJointLookAtController);
    if (ctrl[0x53]) {
        sead::Vector3f lookAtPos;
        lookAtPos.x = 0.0f;
        lookAtPos.y = 0.0f;
        lookAtPos.z = 0.0f;

        if (mHasRequestedLookAt) {
            lookAtPos.set(mRequestedLookAtTrans);
            mHasRequestedLookAt = false;
        } else {
            const sead::Vector3f* target = nullptr;
            bool found = false;

            if (rs::isExistNpcLookPos(mActor)) {
                target = rs::getNpcLookPos(mActor);
                f32 lookAtDist = mLookAtDistance;
                if (al::calcDistanceH(mActor, *target) <= lookAtDist)
                    found = true;
            }

            if (!found) {
                if (mPlayerEyeSensorHitHolder) {
                    auto* sensor = *reinterpret_cast<al::HitSensor* const*>(
                        reinterpret_cast<const u8*>(mPlayerEyeSensorHitHolder) + 0x30);
                    if (sensor) {
                        if (!al::isDead(al::getSensorHost(sensor)) && al::isSensorValid(sensor)) {
                            target = &al::getSensorPos(sensor);
                            f32 lookAtDist = mLookAtDistance;
                            if (al::calcDistanceH(mActor, *target) <= lookAtDist)
                                found = true;
                        }
                    }
                }
            }

            if (found)
                lookAtPos.set(*target);
        }

        mJointLookAtController->requestJointLookAt(lookAtPos);
    }

    ctrl = reinterpret_cast<u8*>(mJointLookAtController);
    bool wasRequested = ctrl[0x50];
    ctrl[0x50] = 0;
    ctrl[0x51] = wasRequested;
}
