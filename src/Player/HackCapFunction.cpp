#include "Player/HackCapFunction.h"

#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"

#include "Player/PlayerColliderHackCap.h"
#include "Player/PlayerInput.h"
#include "Player/PlayerSeparateCapFlag.h"
#include "Util/JudgeUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerUtil.h"

namespace {
// NON_MATCHING: target is 0x134 bytes and current output is 0x138 because the no-wall XY writes are paired only in the target; next source-level hypothesis is the original f32 vector-scaling idiom.
void calcMoveInput(sead::Vector3f* velocity, sead::Vector3f* moveInput,
                   const PlayerColliderHackCap* collider, const sead::Vector3f& input,
                   const sead::Vector3f& up) {
    moveInput->set(input);
    if (rs::isCollidedWall(collider)) {
        al::limitVectorOppositeDir(moveInput, rs::getCollidedWallNormal(collider), *moveInput,
                                   moveInput->length());
        sead::Vector3f reaction(0.0f, 0.0f, 0.0f);
        al::verticalizeVec(&reaction, up, rs::getCollidedFixReaction(collider) * 0.5f);
        *velocity += reaction;
    } else {
        *velocity *= 0.5f;
    }
}

// NON_MATCHING: target is 0x1E0 bytes and current output is 0x1DC because the final X/Y writes are separate only in the target; next source-level hypothesis is the original component-update expression order.
void calcTargetPosition(sead::Vector3f* target, const al::LiveActor* player,
                        const sead::Vector3f& up, const sead::Vector3f& localOffset,
                        f32 height, f32 radius) {
    f32 ceilingSpace = 0.0f;
    rs::tryCalcPlayerCeilingSpace(&ceilingSpace, player, height, radius);
    target->setScaleAdd(ceilingSpace, up, al::getTrans(player));

    sead::Quatf quat = sead::Quatf::unit;
    al::calcQuat(&quat, player);
    sead::Vector3f offset = localOffset;
    offset.rotate(quat);

    f32 shortage = ceilingSpace - height;
    if (shortage < 0.0f) {
        const f32 offsetUp = up.dot(offset);
        if (offsetUp < 0.0f) {
            shortage = sead::Mathf::max(shortage, offsetUp);
            offset -= up * shortage;
        }
    }
    target->y += offset.y;
    target->x += offset.x;
    target->z += offset.z;
}

// NON_MATCHING: exact size, but first mismatch at 0x710040A6AC is parameter-register move ordering; next source-level hypothesis is local reference/value capture order before the quaternion calls.
void updatePose(al::LiveActor* actor, const PlayerColliderHackCap* collider,
                const al::LiveActor* player, const sead::Vector3f& moveInput,
                const sead::Vector3f& up, const sead::Vector3f& velocity,
                const sead::Vector3f& localOffset, f32 height) {
    sead::Quatf actorQuat = sead::Quatf::unit;
    al::calcQuat(&actorQuat, actor);
    sead::Quatf playerQuat = sead::Quatf::unit;
    al::calcQuat(&playerQuat, player);

    sead::Vector3f targetPosition(0.0f, 0.0f, 0.0f);
    calcTargetPosition(&targetPosition, player, up, localOffset, height,
                       collider->getColliderRadius());
    al::setVelocity(actor, targetPosition + velocity - al::getTrans(actor));

    if (al::isNearZero(moveInput, 0.001f)) {
        sead::Vector3f playerFront(0.0f, 0.0f, 0.0f);
        al::calcQuatFront(&playerFront, playerQuat);
        sead::Quatf targetQuat = sead::Quatf::unit;
        if (al::isParallelDirection(up, playerFront, 0.01f))
            targetQuat.set(playerQuat);
        else
            al::makeQuatUpFront(&targetQuat, up, playerFront);

        sead::Quatf quat = sead::Quatf::unit;
        al::slerpQuat(&quat, actorQuat, targetQuat, 0.25f);
        al::updatePoseQuat(actor, quat);
        return;
    }

    sead::Vector3f front = moveInput;
    al::normalize(&front);
    if (al::isParallelDirection(front, up, 0.01f))
        return;

    sead::Quatf targetQuat = sead::Quatf::unit;
    al::makeQuatFrontUp(&targetQuat, front, up);
    al::slerpQuat(&targetQuat, actorQuat, targetQuat, 0.25f);
    al::updatePoseQuat(actor, targetQuat);
}
}  // namespace

namespace HackCapFunction {
void resetPositionAndCollision(al::LiveActor* actor, PlayerColliderHackCap* collider) {
    al::resetPosition(actor);
    rs::resetCollision(collider);
}

bool isKeepSeparateHold(const PlayerSeparateCapFlag* flag, IJudge* judge, bool isHold) {
    bool keepSeparate = false;
    const u32 flags = flag->getRawFlags();
    if (static_cast<u8>(flags))
        keepSeparate = (flags & 0xff0000) == 0;
    if (keepSeparate && isHold)
        return !rs::judgeAndResetReturnTrue(judge);
    return false;
}

f32 calcSeparateJumpGravity(bool* isJumpStart, s32* startCounter, bool* isJumpContinue,
                            s32* jumpCounter, const al::LiveActor* actor,
                            const PlayerColliderHackCap*, const PlayerInput* input, f32 gravity,
                            s32 holdFrame) {
    if (*isJumpStart) {
        if (input->isHoldCapSeparateJump()) {
            *startCounter = al::converge(*startCounter, 0, 1);
            if (*startCounter > 0)
                gravity = 0.0f;
            else
                *isJumpStart = false;
        } else {
            *startCounter = 0;
            *isJumpStart = false;
        }
    }

    if (*jumpCounter >= 1) {
        if (!input->isHoldCapSeparateJump()) {
            *jumpCounter = sead::Mathi::min(*jumpCounter, holdFrame);
            *isJumpContinue = false;
        }
        if (al::isNearZeroOrLess(al::calcSpeedV(actor), 0.001f)) {
            const s32 isContinue = *isJumpContinue;
            const s32 counter = *jumpCounter;
            const s32 nextCounter = al::converge(counter, isContinue, 1);
            gravity = 0.0f;
            *jumpCounter = nextCounter;
        }
    }
    return gravity;
}

void updateSeparateWaitMove(al::LiveActor* actor, sead::Vector3f* velocity,
                            const PlayerColliderHackCap* collider, const al::LiveActor* player,
                            const PlayerInput* input, f32 accel, f32 maxSpeed, f32 height,
                            const sead::Vector3f& localOffset) {
    sead::Vector3f up(0.0f, 0.0f, 0.0f);
    rs::calcPlayerGroundPoseUp(&up, actor);
    sead::Vector3f inputDirection(0.0f, 0.0f, 0.0f);
    input->calcCapSeparateMoveInput(&inputDirection, up);
    sead::Vector3f moveInput(0.0f, 0.0f, 0.0f);
    calcMoveInput(velocity, &moveInput, collider, inputDirection, up);
    *velocity += moveInput * accel;
    al::limitLength(velocity, *velocity, maxSpeed);
    updatePose(actor, collider, player, inputDirection, up, *velocity, localOffset, height);
}

bool updateSeparateWaitJump(al::LiveActor* actor, sead::Vector3f* velocity, f32* jumpSpeed,
                            const PlayerColliderHackCap* collider, const al::LiveActor* player,
                            const PlayerInput* input, f32 accel, f32 maxSpeed, f32 height,
                            f32 gravity, const sead::Vector3f& localOffset) {
    sead::Vector3f up(0.0f, 0.0f, 0.0f);
    al::calcUpDir(&up, actor);
    sead::Vector3f inputDirection(0.0f, 0.0f, 0.0f);
    input->calcCapSeparateMoveInput(&inputDirection, up);

    sead::Vector3f targetPosition(0.0f, 0.0f, 0.0f);
    calcTargetPosition(&targetPosition, player, up, localOffset, height,
                       collider->getColliderRadius());

    sead::Vector3f initialVelocityH(0.0f, 0.0f, 0.0f);
    sead::Vector3f initialVelocityV(0.0f, 0.0f, 0.0f);
    al::separateVectorHV(&initialVelocityH, &initialVelocityV, up,
                         al::getTrans(actor) - targetPosition);
    const f32 verticalDot = up.dot(initialVelocityV);
    const f32 verticalSpeed = verticalDot < 0.0f ? 0.0f : verticalDot;
    velocity->setScaleAdd(verticalSpeed, up, initialVelocityH);

    sead::Vector3f moveInput(0.0f, 0.0f, 0.0f);
    calcMoveInput(velocity, &moveInput, collider, inputDirection, up);
    sead::Vector3f velocityH(0.0f, 0.0f, 0.0f);
    sead::Vector3f velocityV(0.0f, 0.0f, 0.0f);
    al::separateVectorHV(&velocityH, &velocityV, up, *velocity);
    velocityH += moveInput * accel;
    al::limitLength(&velocityH, velocityH, maxSpeed);
    velocityV += up * *jumpSpeed;
    *jumpSpeed -= gravity;

    const bool isJumpEnd = al::isNearZeroOrLess(velocityV.dot(up), 0.001f);
    if (isJumpEnd)
        velocityV.set(0.0f, 0.0f, 0.0f);

    velocity->setAdd(velocityH, velocityV);
    updatePose(actor, collider, player, inputDirection, up, *velocity, localOffset, height);
    return isJumpEnd;
}
}  // namespace HackCapFunction
