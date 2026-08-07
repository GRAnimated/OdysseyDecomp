#include "Player/HackCapStateThrowStay.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/HackCapFunction.h"
#include "Player/HackCapJudgePreInputSeparateJump.h"
#include "Player/HackCapJointControlKeeper.h"
#include "Player/HackCapTrigger.h"
#include "Player/PlayerColliderHackCap.h"
#include "Player/PlayerEyeSensorHitHolder.h"
#include "Player/PlayerInput.h"
#include "Player/PlayerSeparateCapFlag.h"
#include "Util/JudgeUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(HackCapStateThrowStay, Stay)
NERVE_IMPL_(HackCapStateThrowStay, SeparateDirectPlayer, SeparateHomingAttack)
NERVE_IMPL(HackCapStateThrowStay, SeparateHipDropLoop)
NERVE_IMPL(HackCapStateThrowStay, SeparateJump)
NERVE_IMPL_(HackCapStateThrowStay, SeparateHomingPlayer, SeparateHomingAttack)
NERVE_IMPL(HackCapStateThrowStay, SeparateMove)
NERVE_IMPL(HackCapStateThrowStay, SeparateHipDropStart)
NERVE_IMPL(HackCapStateThrowStay, SeparateHomingAttack)
NERVE_IMPL_(HackCapStateThrowStay, SeparateJumpRestart, SeparateJump)
NERVE_IMPL(HackCapStateThrowStay, SeparateHipDropLand)
NERVE_IMPL(HackCapStateThrowStay, SeparateApproach)
NERVE_IMPL(HackCapStateThrowStay, SeparateApproachEnd)
NERVES_MAKE_STRUCT(HackCapStateThrowStay, Stay, SeparateDirectPlayer, SeparateHipDropLoop,
                   SeparateJump, SeparateHomingPlayer, SeparateMove, SeparateHipDropStart,
                   SeparateHomingAttack, SeparateJumpRestart, SeparateHipDropLand)
NERVES_MAKE_NOSTRUCT(HackCapStateThrowStay, SeparateApproach)
NERVES_MAKE_NOSTRUCT(HackCapStateThrowStay, SeparateApproachEnd)
}  // namespace

HackCapStateThrowStay::HackCapStateThrowStay(
    al::LiveActor* actor, const PlayerColliderHackCap* collider, const al::LiveActor* player,
    const PlayerSeparateCapFlag* separateCapFlag, const PlayerInput* input,
    const IUsePlayerCollision* collision, const PlayerEyeSensorHitHolder* eyeSensorHitHolder,
    const HackCapTrigger* trigger, HackCapJointControlKeeper* jointControlKeeper,
    HackCapJudgePreInputSeparateJump* judgePreInputSeparateJump, const bool* isDirectPlayer)
    : ActorStateBase("投げ中待機", actor), mCollider(collider), mPlayer(player),
      mSeparateCapFlag(separateCapFlag), mInput(input), mCollision(collision),
      mEyeSensorHitHolder(eyeSensorHitHolder), mTrigger(trigger),
      mJointControlKeeper(jointControlKeeper),
      mJudgePreInputSeparateJump(judgePreInputSeparateJump), _68(reinterpret_cast<const u8*>(isDirectPlayer)), _70(false),
      _71(false), _72(false), _74{0.0f, 0.0f, 0.0f}, _80{0.0f, 0.0f, 0.0f},
      _8c{0.0f, 0.0f, 0.0f}, _98(0.0f), _9c(false), _a0{0.0f, 0.0f, 0.0f},
      _b0(nullptr), _b8(0.0f), _bc(0.0f), _c0(false), _c4(0), _c8(false), _cc(0),
      _d0(0), _e0(nullptr), _e8{0.0f, 0.0f, 0.0f}, _f4(0), _f8(0.0f), _fc{0.0f, 0.0f, 0.0f}, _108(0),
      _10c(false) {
    initNerve(&NrvHackCapStateThrowStay.Stay, 0);
}

HackCapStateThrowStay::~HackCapStateThrowStay() = default;

void HackCapStateThrowStay::appear() {
    al::LiveActor* actor = mActor;
    al::NerveStateBase::appear();
    _e0 = nullptr;
    _80.set(0.0f, 0.0f, 0.0f);
    _8c.set(0.0f, 0.0f, 0.0f);
    _98 = 0.0f;
    _9c = false;
    al::setVelocityZero(actor);
    _70 = true;

    if (mTrigger->isOn(HackCapTrigger::Trigger1)) {
        al::setNerve(this, &NrvHackCapStateThrowStay.SeparateDirectPlayer);
        return;
    }

    if (*_68) {
        const u32 flags = mSeparateCapFlag->getRawFlags();
        if ((flags & 0xFF0000) == 0 && (flags & 0xFF) != 0 &&
            mTrigger->isOn(HackCapTrigger::Trigger2)) {
            sead::Vector3f up = {0.0f, 0.0f, 0.0f};
            if (rs::isCollidedGround(mCollider))
                up = rs::getCollidedGroundNormal(mCollider);
            else
                up = -al::getGravity(actor);
            sead::Vector3f input = {0.0f, 0.0f, 0.0f};
            mInput->calcCapSeparateMoveInput(&input, up);
            al::setVelocity(actor, 35.0f * input);
            _70 = false;
        }
    }
    al::setNerve(this, &NrvHackCapStateThrowStay.Stay);
}

void HackCapStateThrowStay::kill() {
    al::NerveStateBase::kill();
    mJointControlKeeper->resetRotateY();
}

bool HackCapStateThrowStay::update() {
    const al::LiveActor* actor = mActor;
    const PlayerSeparateCapFlag* separateCapFlag = mSeparateCapFlag;
    const bool directPlayer = *_68 != 0;
    const bool isStay = al::isNerve(this, &NrvHackCapStateThrowStay.Stay);
    const u32 flags = separateCapFlag->getRawFlags();
    bool isInvalidSeparate = false;
    if (directPlayer)
        isInvalidSeparate = true;
    if ((flags & 0xFF0000) != 0)
        isInvalidSeparate = true;
    if ((flags & 0xFF) == 0)
        isInvalidSeparate = true;


    if (isStay) {
        if (!isInvalidSeparate) {
            sead::Vector3f* offset = &_8c;
            const sead::Vector3f& trans = al::getTrans(actor);
            offset->setSub(trans, _74);
            al::verticalizeVec(offset, al::getGravity(actor), *offset);
            const sead::Vector3f& verticalTrans = al::getTrans(actor);
            _80.setSub(verticalTrans, *offset);
            al::setNerve(this, &NrvHackCapStateThrowStay.SeparateMove);
        }
    } else if (isInvalidSeparate) {
        al::setNerve(this, &NrvHackCapStateThrowStay.Stay);
    }

    if (mTrigger->isOn(HackCapTrigger::Trigger3) &&
        al::isNerve(this, &NrvHackCapStateThrowStay.SeparateHipDropLoop))
        al::setNerve(this, &NrvHackCapStateThrowStay.SeparateJump);

    return al::NerveStateBase::update();
}

bool HackCapStateThrowStay::isHomingPlayerJump() const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvHackCapStateThrowStay.SeparateHomingPlayer))
        return true;
    return al::isNerve(this, &NrvHackCapStateThrowStay.SeparateDirectPlayer);
}

bool HackCapStateThrowStay::isEnableAppendAttack() const {
    if (isDead())
        return true;
    if (al::isNerve(this, &NrvHackCapStateThrowStay.Stay))
        return true;
    if (al::isNerve(this, &NrvHackCapStateThrowStay.SeparateMove))
        return _8c.length() <= 2000.0f;
    return false;
}

bool HackCapStateThrowStay::isEnableKeepStayTouchJump() const {
    return !isDead() &&
           (al::isNerve(this, &NrvHackCapStateThrowStay.SeparateHomingPlayer) ||
            al::isNerve(this, &NrvHackCapStateThrowStay.SeparateDirectPlayer)) &&
           al::isNerve(this, &NrvHackCapStateThrowStay.SeparateHomingPlayer);
}

bool HackCapStateThrowStay::isEnableTouchJumpTransWarp() const {
    return !isDead() &&
           (al::isNerve(this, &NrvHackCapStateThrowStay.SeparateHomingPlayer) ||
            al::isNerve(this, &NrvHackCapStateThrowStay.SeparateDirectPlayer)) &&
           al::isNerve(this, &NrvHackCapStateThrowStay.SeparateDirectPlayer) &&
           rs::isPlayerInWater(mActor);
}

bool HackCapStateThrowStay::isEnableSendHipDropMsg() const {
    if (isDead())
        return false;
    const u32 flags = mSeparateCapFlag->getRawFlags();
    if ((flags & 0xFF0000) != 0 || (flags & 0xFF) == 0)
        return false;
    if (al::isNerve(this, &NrvHackCapStateThrowStay.SeparateHipDropLoop))
        return true;
    if (!al::isNerve(this, &NrvHackCapStateThrowStay.SeparateHipDropStart))
        return false;
    return al::isGreaterStep(this, 25);
}

bool HackCapStateThrowStay::sendHipDropCollideMsg(al::HitSensor* sender) {
    if (!rs::isCollidedGround(mCollider))
        return false;
    al::HitSensor* receiver = rs::tryGetCollidedGroundSensor(mCollider);
    if (!rs::sendMsgCapHipDrop(receiver, sender))
        return false;
    _10c = mInput->isHoldCapSeparateHipDrop();
    return true;
}

bool HackCapStateThrowStay::sendHipDropObjMsg(HackCapTrigger* trigger, al::HitSensor* receiver,
                                               al::HitSensor* sender) {
    if (!rs::sendMsgCapObjHipDropReflect(receiver, sender))
        return rs::sendMsgCapObjHipDrop(receiver, sender);
    trigger->set(HackCapTrigger::Trigger3);
    return true;
}

void HackCapStateThrowStay::exeStay() {}

namespace {
// NON_MATCHING: exact 600-byte size, but target uses a 0xD0 frame with D15-D8 saves and
// S0/S1 spills versus current 0xB0/D11-D8; next keep scalar args addressable and split vector
// components to extend floating-point lifetimes.
bool tryEndSeparateJump(al::LiveActor* actor, const IUsePlayerCollision* collision,
                        const sead::Vector3f& stayPos, const sead::Vector3f& jumpStartPos,
                        const sead::Vector3f& velocityH, f32 moveLimit, f32 gravityAccel) {
    const sead::Vector3f& gravity = al::getGravity(actor);
    const sead::Vector3f& trans = al::getTrans(actor);
    if (rs::isOnGround(actor, collision))
        return true;

    const f32 distanceV = (jumpStartPos - trans).dot(gravity);
    if (al::isNearZeroOrLess(distanceV, 0.001f) && al::calcSpeedV(actor) < 0.0f)
        return true;

    al::addVelocityToGravityLimit(actor, gravityAccel, 40.0f);
    sead::Vector3f actorVelocityH = sead::Vector3f::zero;
    sead::Vector3f actorVelocityV = sead::Vector3f::zero;
    al::separateVelocityHV(&actorVelocityH, &actorVelocityV, actor);

    sead::Vector3f stayDirH = sead::Vector3f::zero;
    al::verticalizeVec(&stayDirH, gravity, stayPos - trans);
    f32 approachSpeed = stayDirH.length() - 2000.0f;
    if (approachSpeed < 0.0f)
        approachSpeed = 0.0f;
    else if (approachSpeed > moveLimit)
        approachSpeed = moveLimit;
    al::limitLength(&stayDirH, stayDirH, approachSpeed);

    if (distanceV > 0.0f && al::isNearZero(gravity, 0.001f))
        actorVelocityV.set(0.0f, 0.0f, 0.0f);
    al::setVelocity(actor, stayDirH + velocityH + actorVelocityV);
    return false;
}
}  // namespace

void HackCapStateThrowStay::updateStayMove() {
    al::LiveActor* actor = mActor;
    sead::Vector3f up(0.0f, 0.0f, 0.0f);
    if (rs::isCollidedGround(mCollider))
        up = rs::getCollidedGroundNormal(mCollider);
    else
        up = -al::getGravity(actor);

    sead::Vector3f moveInput(0.0f, 0.0f, 0.0f);
    mInput->calcCapSeparateMoveInput(&moveInput, up);
    sead::Vector3f moveDir(0.0f, 0.0f, 0.0f);
    sead::Vector3f moveVelocity(0.0f, 0.0f, 0.0f);
    if (al::tryNormalizeOrZero(&moveDir, moveInput)) {
        const f32 inputLength = moveInput.length();
        const f32 maxSpeed = _71 ? 15.0f : sead::Mathf::clampMin(_a0.y, 35.0f);
        const f32 targetSpeed = inputLength * maxSpeed;
        const f32 currentSpeed =
            sead::Mathf::clampMin(sead::Mathf::abs(moveDir.dot(al::getVelocity(actor))), 0.0f);
        const f32 accel = inputLength * (_71 ? 1.5f : 3.0f) + currentSpeed;
        moveVelocity = moveDir * (accel > targetSpeed ? targetSpeed : accel);
    }

    sead::Vector3f stayMove = _8c + moveVelocity;
    const f32 stayMoveLimit =
        sead::Mathf::clampMin(stayMove.length() - (_a0.z + 2000.0f), 0.0f) + 2000.0f;
    al::limitLength(&stayMove, stayMove, stayMoveLimit);

    sead::Vector3f velocity = _80 + stayMove - al::getTrans(actor);
    al::limitLength(&velocity, velocity, _a0.z);
    if (rs::isCollidedGround(mCollider)) {
        const sead::Vector3f& normal = rs::getCollidedGroundNormal(mCollider);
        al::limitVectorOppositeDir(&velocity, normal, velocity, velocity.length());
        if (al::isNearZeroOrLess(normal.dot(velocity), 0.001f))
            velocity -= normal * 10.0f;
    }
    if (rs::isCollidedWall(mCollider)) {
        const sead::Vector3f& normal = rs::getCollidedWallNormal(mCollider);
        al::limitVectorOppositeDir(&velocity, normal, velocity, velocity.length());
    }
    if (rs::isCollidedCeiling(mCollider)) {
        const sead::Vector3f& normal = rs::getCollidedCeilingNormal(mCollider);
        al::limitVectorOppositeDir(&velocity, normal, velocity, velocity.length());
    }
    al::setVelocity(actor, velocity);
}

void HackCapStateThrowStay::exeSeparateJump() {
    al::LiveActor* actor = mActor;
    f32 jumpSpeed = 15.0f;
    const sead::Vector3f& gravity = al::getGravity(actor);
    const f32 moveScale = _71 ? 1.0f : 1.5f;
    const f32 moveLimit = _71 ? 10.0f : 15.0f;
    sead::Vector3f moveInput;
    sead::Vector3f temp;
    bool* isHoldJump;
    s32* holdCounter;
    bool* isJump;
    s32* jumpCounter;
    sead::Vector3f* moveVelocity;
    if (al::isFirstStep(this)) {
        al::startHitReaction(actor, "おすそ分けジャンプ");
        if (al::isNerve(this, &NrvHackCapStateThrowStay.SeparateJump))
            _e8 = al::getTrans(actor);

        const bool isAfterMovement = mTrigger->isOn(HackCapTrigger::Trigger3);
        isHoldJump = &_c0;
        s32 holdCount;
        f32 jumpGravity;
        if (_71) {
            holdCount = 0;
            *isHoldJump = false;
            if (!isAfterMovement)
                jumpSpeed = 10.0f;
            jumpGravity = 0.5f;
        } else {
            *isHoldJump = true;
            jumpSpeed = isAfterMovement ? 25.0f : 18.0f;
            jumpGravity = isAfterMovement ? 1.5f : 2.0f;
            holdCount = isAfterMovement ? 0 : 10;
        }
        _c4 = holdCount;
        _c8 = false;
        _cc = 20;
        _bc = jumpGravity;
        holdCounter = &_c4;
        isJump = &_c8;
        jumpCounter = &_cc;
        moveVelocity = &_fc;

        moveInput = sead::Vector3f(0.0f, 0.0f, 0.0f);
        al::separateVelocityHV(moveVelocity, &moveInput, actor);
        moveInput.setScale(gravity, -jumpSpeed);
        al::limitLength(moveVelocity, *moveVelocity, moveLimit);
        temp = *moveVelocity + moveInput;
        al::setVelocity(actor, temp);
    } else {
        isHoldJump = &_c0;
        holdCounter = &_c4;
        isJump = &_c8;
        jumpCounter = &_cc;
        moveVelocity = &_fc;
    }

    const f32 jumpGravity = HackCapFunction::calcSeparateJumpGravity(
        isHoldJump, holdCounter, isJump, jumpCounter, actor, mCollider, mInput, _bc, 5);
    const PlayerInput* input = mInput;
    moveInput = sead::Vector3f(0.0f, 0.0f, 0.0f);
    temp = -al::getGravity(actor);
    input->calcCapSeparateMoveInput(&moveInput, temp);
    temp = moveInput * moveScale;
    al::addVectorLimit(moveVelocity, temp, moveLimit);
    al::limitLength(moveVelocity, *moveVelocity, moveLimit);

    if (rs::judgeAndResetReturnTrue(mJudgePreInputSeparateJump) &&
        al::getGravity(actor).dot(_74 - al::getTrans(actor)) < 500.0f) {
        al::setNerve(this, &NrvHackCapStateThrowStay.SeparateJumpRestart);
    } else if (tryEndSeparateJump(actor, mCollider, _74, _e8, *moveVelocity, _a0.z,
                                      jumpGravity)) {
        al::setNerve(this, &NrvHackCapStateThrowStay.SeparateMove);
    } else if (mInput->isTriggerCapSeparateHipDrop()) {
        al::setNerve(this, &NrvHackCapStateThrowStay.SeparateHipDropStart);
    }
}

void HackCapStateThrowStay::exeSeparateHipDropStart() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        _10c = false;
        al::setVelocityZero(actor);
        al::startHitReaction(actor, "おすそ分けヒップドロップ");
    }
    mJointControlKeeper->setRotateY(al::calcNerveRate(this, 10) * 360.0f);
    if (!al::isLessEqualStep(this, 25)) {
        mJointControlKeeper->resetRotateY();
        if (rs::isCollidedGround(mCollider) && !_10c) {
            al::startHitReaction(actor, "おすそ分けヒップドロップ着地");
            al::setNerve(this, &NrvHackCapStateThrowStay.SeparateHipDropLand);
        } else {
            al::setNerve(this, &NrvHackCapStateThrowStay.SeparateHipDropLoop);
        }
    }
}

void HackCapStateThrowStay::exeSeparateHipDropLoop() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        _10c = false;
        al::setVelocityToGravity(actor, _71 ? 25.0f : 45.0f);
        _108 = _71 ? 10 : 0;
    }
    if (al::isGreaterEqualStep(this, 15)) {
        if (mInput->isHoldCapSeparateHipDrop())
            _108 = al::converge(_108, 0, 1);
        else
            _108 = 0;
    }
    if (_71 && _108 == 0)
        al::scaleVelocity(actor, 0.875f);

    if (rs::isCollidedGround(mCollider) || al::isGreaterEqualStep(this, 60) ||
        ((_71 || _72) && al::calcSpeedV(actor) > -1.25f)) {
        if (_10c) {
            al::setNerve(this, &NrvHackCapStateThrowStay.SeparateHipDropLoop);
        } else {
            if (rs::isCollidedGround(mCollider))
                al::startHitReaction(actor, "おすそ分けヒップドロップ着地");
            al::setNerve(this, &NrvHackCapStateThrowStay.SeparateHipDropLand);
        }
    }
}

void HackCapStateThrowStay::exeSeparateHipDropLand() {
    if (al::isFirstStep(this)) {
        al::startAction(mActor, "SeparateHipDropLand");
        al::setVelocityZero(mActor);
    }
    if (al::isGreaterEqualStep(this, 5) && rs::judgeAndResetReturnTrue(mJudgePreInputSeparateJump)) {
        al::startAction(mActor, _b0);
        al::setNerve(this, &NrvHackCapStateThrowStay.SeparateJump);
    } else if (al::isActionEnd(mActor)) {
        al::startAction(mActor, _b0);
        al::setNerve(this, &NrvHackCapStateThrowStay.SeparateMove);
    }
}

void HackCapStateThrowStay::exeSeparateFallDown() {
    al::LiveActor* actor = mActor;
    const sead::Vector3f& gravity = al::getGravity(actor);
    if (al::isFirstStep(this)) {
        f32 speed = gravity.dot(al::getVelocity(actor));
        if (speed < 30.0f)
            speed = 30.0f;
        else if (speed > 45.0f)
            speed = 45.0f;
        al::setVelocity(actor, speed * gravity);
        al::startHitReaction(actor, "おすそ分けヒップドロップ");
    } else {
        al::addVelocityToGravityLimit(actor, 1.0f, 45.0f);
    }
    if (!mInput->isHoldCapSeparateHipDrop() || rs::isCollidedGround(mCollider))
        al::setNerve(this, &NrvHackCapStateThrowStay.SeparateMove);
}

void HackCapStateThrowStay::exeSeparateApproachStart() {
    const al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        _b8 = 18.0f;
        al::startHitReaction(actor, "おすそ分けジャンプ");
    }
    const sead::Vector3f up = -al::getGravity(actor);
    sead::Vector3f vertical = {0.0f, 0.0f, 0.0f};
    al::parallelizeVec(&vertical, up, al::getTrans(actor) - _74);
    vertical += _b8 * up;
    _80.setAdd(_74, vertical);
    al::verticalizeVec(&_8c, up, al::getTrans(actor) - _74);
    _b8 -= 2.0f;
    updateStayMove();
    if (_b8 < 0.0f)
        al::setNerve(this, &SeparateApproach);
}

void HackCapStateThrowStay::exeSeparateApproach() {
    const al::LiveActor* actor = mActor;
    const sead::Vector3f& gravity = al::getGravity(actor);
    if (al::isFirstStep(this)) {
        sead::Vector3f vertical = {0.0f, 0.0f, 0.0f};
        al::parallelizeVec(&vertical, gravity, al::getTrans(actor) - _74);
        const f32 verticalDistance = vertical.dot(gravity);
        const f32 distance = sead::Mathf::abs(_a0.x + verticalDistance);
        const f32 approachFrames = distance / 30.0f;
        _d0 = sead::Mathf::ceil(approachFrames < 5.0f ? 5.0f : approachFrames);
        _d4 = al::getTrans(actor);
    }
    const sead::Vector3f prevTarget = _80;
    sead::Vector3f startVertical = {0.0f, 0.0f, 0.0f};
    al::parallelizeVec(&startVertical, gravity, _d4 - _74);
    f32 rate = static_cast<f32>(al::getNerveStep(this) + 1) / static_cast<f32>(_d0);
    rate = sead::Mathf::clamp(rate, 0.0f, 1.0f);
    rate = al::easeIn(rate);
    const f32 startVerticalDistance = gravity.dot(startVertical);
    const f32 distance = al::lerpValue(startVerticalDistance, -_a0.x, rate);
    _80 = _74 + distance * gravity;
    al::verticalizeVec(&_8c, gravity, al::getTrans(actor) - _74);
    const f32 delta = (_80 - prevTarget).dot(gravity);
    f32 correction;
    if (delta > 30.0f)
        correction = -30.0f;
    else if (delta < -30.0f)
        correction = 30.0f;
    else
        correction = -delta;
    _b8 = correction;
    updateStayMove();
    if (al::isGreaterEqualStep(this, _d0 - 1))
        al::setNerve(this, &SeparateApproachEnd);
}

void HackCapStateThrowStay::exeSeparateApproachEnd() {
    const al::LiveActor* actor = mActor;
    const sead::Vector3f& gravity = al::getGravity(actor);
    sead::Vector3f vertical = {0.0f, 0.0f, 0.0f};
    al::parallelizeVec(&vertical, gravity, al::getTrans(actor) - _74);
    const f32 rate = al::easeOut(al::calcNerveRate(this, 30));
    const f32 verticalDistanceXY = gravity.x * vertical.x + gravity.y * vertical.y;
    const f32 verticalDistanceZ = gravity.z * vertical.z;
    const f32 oneMinusRate = 1.0f - rate;
    const f32 verticalDistance = verticalDistanceXY + verticalDistanceZ;
    const f32 start = verticalDistance - oneMinusRate * _b8;
    _80 = _74 + al::lerpValue(start, -_a0.x, rate) * gravity;
    al::verticalizeVec(&_8c, gravity, al::getTrans(actor) - _74);
    updateStayMove();
    if (al::isGreaterEqualStep(this, 30)) {
        _b8 = 0.0f;
        al::setNerve(this, &NrvHackCapStateThrowStay.SeparateMove);
    }
}

// NON_MATCHING: 1484 bytes versus the 1408-byte target; next split the approach/brake decision tree and shorten search-vector lifetimes.
void HackCapStateThrowStay::exeSeparateMove() {
    if (al::isFirstStep(this))
        _98 = 0.0f;

    sead::Vector3f playerUp = sead::Vector3f::zero;
    rs::calcGroundNormalOrUpDir(&playerUp, mPlayer, mCollision);
    const f32 baseHeight = _a0.x;
    f32 groundHeight = baseHeight * (-al::getGravity(mPlayer)).dot(playerUp);
    groundHeight = sead::Mathf::clamp(groundHeight, 0.0f, baseHeight);

    sead::Vector3f vertical = sead::Vector3f::zero;
    al::parallelizeVec(&vertical, al::getGravity(mActor), al::getTrans(mActor) - _74);
    const f32 verticalLength = vertical.length();
    const bool playerAir = !rs::isPlayerCollidedGround(mPlayer);
    const f32 verticalDistance = vertical.dot(al::getGravity(mPlayer)) - groundHeight;
    const bool shouldApproach =
        _9c ? !al::isNearZeroOrLess(verticalDistance, 0.001f) :
              playerAir && verticalDistance > (_71 ? 200.0f : 500.0f);
    _9c = shouldApproach;

    f32 brakeTarget = 0.0f;
    const bool approachOrFast = shouldApproach || _71;
    if ((!playerAir || approachOrFast) &&
        (verticalDistance >= 0.0f || !rs::isCollidedGround(mCollider))) {
        if (verticalDistance <= 0.0f ||
            (approachOrFast && !rs::isCollidedCeiling(mCollider))) {
            brakeTarget = sead::Mathf::min(sead::Mathf::abs(verticalDistance) * 0.03f, 40.0f);
        }
    }
    _98 = sead::Mathf::min(_98 + 3.0f, brakeTarget);
    al::limitLength(&vertical, vertical,
                    sead::Mathf::max(verticalLength - _98, groundHeight));
    _80 = _74 + vertical;
    al::verticalizeVec(&_8c, al::getGravity(mActor), al::getTrans(mActor) - _74);
    updateStayMove();

    if (rs::judgeAndResetReturnTrue(mJudgePreInputSeparateJump)) {
        const sead::Vector3f up = -al::getGravity(mPlayer);
        sead::Vector3f searchDir = sead::Vector3f::zero;
        mInput->calcCapSeparateMoveInput(&searchDir, up);
        bool useWideSearch = false;
        if (!al::tryNormalizeOrZero(&searchDir)) {
            al::verticalizeVec(&searchDir, up, al::getTrans(mActor) - _74);
            if (!al::tryNormalizeOrZero(&searchDir)) {
                _e0 = nullptr;
            } else {
                useWideSearch = true;
            }
        }

        if (!al::isNearZero(searchDir, 0.001f)) {
            sead::Vector3f targetDir = sead::Vector3f::zero;
            _e0 = mEyeSensorHitHolder->findNearestSensorLimit(
                &targetDir, al::getTrans(mActor), searchDir, up, 250.0f, 45.0f, 80.0f,
                useWideSearch ? 300.0f : 500.0f);
            if (!_e0 && useWideSearch) {
                _e0 = mEyeSensorHitHolder->findNearestSensorLimit(
                    &targetDir, al::getTrans(mActor), searchDir, up, 250.0f, 180.0f, 180.0f,
                    300.0f);
            }
        }

        if (_e0) {
            al::setNerve(this, &NrvHackCapStateThrowStay.SeparateHomingAttack);
            return;
        }

        bool jumpAway = _8c.length() >= 300.0f;
        if (!jumpAway) {
            sead::Vector3f input = sead::Vector3f::zero;
            mInput->calcCapSeparateMoveInput(&input, up);
            sead::Vector3f stayDir = _8c;
            if (al::tryNormalizeOrZero(&input) && al::tryNormalizeOrZero(&stayDir) &&
                input.dot(stayDir) > -0.70711f) {
                jumpAway = true;
            }
        }
        if (jumpAway)
            al::setNerve(this, &NrvHackCapStateThrowStay.SeparateJump);
        else
            al::setNerve(this, &NrvHackCapStateThrowStay.SeparateHomingPlayer);
    } else if (mInput->isTriggerCapSeparateHipDrop()) {
        al::setNerve(this, &NrvHackCapStateThrowStay.SeparateHipDropStart);
    }
}

// NON_MATCHING: 1492 bytes versus the 1536-byte target; next recover the corpus branch ordering around sensor validation and homing termination.
void HackCapStateThrowStay::exeSeparateHomingAttack() {
    const sead::Vector3f& gravity = al::getGravity(mActor);
    if (al::isFirstStep(this)) {
        _e8 = al::getTrans(mActor);
        al::startHitReaction(mActor, "おすそ分けジャンプ");
        _f4 = 1;
    }

    if (al::isLessEqualStep(this, _f4)) {
        const sead::Vector3f& target =
            al::isNerve(this, &NrvHackCapStateThrowStay.SeparateHomingAttack) ?
                al::getSensorPos(_e0) :
                rs::getPlayerBodyPos(mPlayer);
        sead::Vector3f horizontal = sead::Vector3f::zero;
        sead::Vector3f vertical = sead::Vector3f::zero;
        al::separateVectorHV(&horizontal, &vertical, gravity, target - _e8);
        al::limitLength(&horizontal, horizontal, 500.0f);
        const f32 horizontalLength = horizontal.length();
        const f32 verticalLimit = (sead::Mathf::max(500.0f - horizontalLength, 0.0f) + 500.0f) * 0.3f;
        al::limitLength(&vertical, vertical, verticalLimit);
        f32 verticalLength = vertical.length();
        const f32 horizontalArch = sead::Mathf::max(horizontalLength * 0.3f, 200.0f);
        f32 arch = horizontalArch + verticalLength * 0.5f;
        if (arch > verticalLimit) {
            if (vertical.dot(gravity) <= 0.0f) {
                al::limitLength(&vertical, vertical,
                                sead::Mathf::max(verticalLength - (arch - verticalLimit), 0.0f));
                verticalLength = vertical.length();
                arch = horizontalArch + verticalLength * 0.5f;
            } else {
                arch = verticalLimit;
            }
        }
        _f8 = arch;
        if (al::isFirstStep(this)) {
            const f32 distance = sead::Mathf::max(horizontalLength, arch / 0.3f);
            _f4 = sead::Mathf::ceil(distance / (_71 ? 20.0f : 30.0f));
        }

        const f32 rate = al::calcNerveRate(this, _f4);
        const f32 arcOffset = _f8 * sead::Mathf::sin(sead::Mathf::deg2rad(rate * 180.0f));
        const sead::Vector3f desired = _e8 + rate * horizontal + rate * vertical - arcOffset * gravity;
        sead::Vector3f move = desired - al::getTrans(mActor);
        al::limitLength(&move, move, 60.0f);
        if (rs::isCollidedWall(mCollider)) {
            const sead::Vector3f& normal = rs::getCollidedWallNormal(mCollider);
            al::limitVectorOppositeDir(&move, move, normal, move.length());
        }
        if (rs::isCollidedCeiling(mCollider)) {
            const sead::Vector3f& normal = rs::getCollidedCeilingNormal(mCollider);
            al::limitVectorOppositeDir(&move, move, normal, move.length());
        }
        al::setVelocity(mActor, move);
        al::verticalizeVec(&_fc, gravity, move);
    } else {
        if (al::isNerve(this, &NrvHackCapStateThrowStay.SeparateDirectPlayer)) {
            kill();
            return;
        }

        const f32 accelH = _71 ? 1.0f : 1.5f;
        const f32 speedH = _71 ? 10.0f : 15.0f;
        sead::Vector3f input = sead::Vector3f::zero;
        mInput->calcCapSeparateMoveInput(&input, -gravity);
        al::addVectorLimit(&_fc, accelH * input, speedH);
        al::limitLength(&_fc, _fc, speedH);
        if (tryEndSeparateJump(mActor, mCollision, _74, _e8, _fc, _a0.z, 2.0f)) {
            al::setNerve(this, &NrvHackCapStateThrowStay.SeparateMove);
            return;
        }
    }

    if (!al::isNerve(this, &NrvHackCapStateThrowStay.SeparateDirectPlayer) &&
        mInput->isTriggerCapSeparateHipDrop()) {
        al::setNerve(this, &NrvHackCapStateThrowStay.SeparateHipDropStart);
    }
}
