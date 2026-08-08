#include "Player/PlayerStatePoleClimb.h"

#include "Library/Collision/Collider.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/PlayerActionCollisionSnap.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerInput.h"
#include "Player/PlayerJointParamHandLegAngle.h"
#include "Player/PlayerJudgePreInputPoleClimbSwing.h"
#include "Player/PlayerStateNormalWallJump.h"
#include "Util/InputInterruptTutorialUtil.h"
#include "Util/ObjUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/JudgeUtil.h"

namespace {
NERVE_IMPL(PlayerStatePoleClimb, Start)
NERVE_IMPL(PlayerStatePoleClimb, Jump)
NERVE_IMPL(PlayerStatePoleClimb, TopJump)
NERVE_IMPL(PlayerStatePoleClimb, Up)
NERVE_IMPL(PlayerStatePoleClimb, Turn)
NERVE_IMPL(PlayerStatePoleClimb, TopWait)
NERVE_IMPL(PlayerStatePoleClimb, TopTurn)
NERVE_IMPL(PlayerStatePoleClimb, TopStart)
NERVE_IMPL(PlayerStatePoleClimb, TopEnd)
NERVE_IMPL(PlayerStatePoleClimb, Wait)
NERVE_IMPL(PlayerStatePoleClimb, Down)

NERVES_MAKE_STRUCT(PlayerStatePoleClimb, Start, Jump, TopJump, Up, Turn, TopWait, TopTurn,
                   TopStart, TopEnd, Wait, Down)
s32 calcPoleMoveDirection(const PlayerInput* input, const sead::Vector3f& up,
                          const sead::Vector3f& side, const IJudge* judge, s32 moveDirection,
                          bool enableDown, f32 inputX, f32 inputY, f32 inputDegree);

}  // namespace

PlayerStatePoleClimb::PlayerStatePoleClimb(
    al::LiveActor* player, const PlayerConst* pConst, const PlayerInput* input,
    const PlayerTrigger* trigger, PlayerModelHolder* modelHolder, IUsePlayerCollision* collision,
    PlayerAnimator* animator, PlayerWallActionHistory* wallActionHistory,
    PlayerJointParamHandLegAngle* handLegAngle, PlayerJudgePreInputJump* judgePreInputJump,
    PlayerActionDiveInWater* actionDiveInWater)
    : al::ActorStateBase("ポールのぼり", player), mConst(pConst), mInput(input),
      mModelHolder(modelHolder), mCollision(collision), mAnimator(animator),
      mHandLegAngle(handLegAngle), mJudgePreInputJump(judgePreInputJump), mWallJump(nullptr),
      mTopJump(nullptr), mWallActionHistory(wallActionHistory), mCollisionSnap(nullptr),
      mJudgePreInputPoleClimbSwing(nullptr), mMaterialCode("NoCollide"), _88(0.0f), _8c(0.0f),
      _90(0.0f), _98(0), _9c{0.0f, 0.0f}, _a4(true), _a5(false), _a8{0.0f, 0.0f},
      _b0(0), _b4(false), _b8{0.0f, 0.0f} {
    mWallJump = new PlayerStateNormalWallJump(player, pConst, input, collision, trigger, animator,
                                               actionDiveInWater);
    mTopJump = new PlayerStateNormalWallJump(player, mConst, input, collision, trigger, animator,
                                              actionDiveInWater);
    mTopJump->mAnimationName = "PoleHandStandJump";
    mTopJump->mIsJumpTowardsWall = true;
    mCollisionSnap = new PlayerActionCollisionSnap(player, collision);
    mJudgePreInputPoleClimbSwing = new PlayerJudgePreInputPoleClimbSwing(mConst, input);
    initNerve(&NrvPlayerStatePoleClimb.Start, 2);
    al::initNerveState(this, mWallJump, &NrvPlayerStatePoleClimb.Jump, "ジャンプ");
    al::initNerveState(this, mTopJump, &NrvPlayerStatePoleClimb.TopJump, "頂上ジャンプ");
}

// NON_MATCHING: target/current are 404/604 after the validator-required sead dot rewrite made the shared classifier cheap enough to inline again. Next recover a validator-accepted classifier shape near the target 376-byte cost so this caller remains out-of-line without attributes.
void PlayerStatePoleClimb::appear() {
    al::NerveStateBase::appear();
    mCollisionSnap->start();
    mAnimator->startAnim("PoleCatch");
    mHandLegAngle->blendRate = 1.0f;
    rs::resetJudge(reinterpret_cast<IJudge*>(mJudgePreInputPoleClimbSwing));
    _9c = {0.0f, 0.0f};
    _a4 = true;
    _a5 = false;
    _a8 = {0.0f, 0.0f};
    _b0 = 0;
    rs::invalidatePlayerGroundShadow(mModelHolder);
    rs::tryAppearPlayerClimbPoleTutorial(mActor);

    _b4 = false;
    _b8 = {0.0f, 0.0f};
    sead::Vector2f poleMoveInput = {0.0f, 0.0f};
    const PlayerInput* input = mInput;
    const f32 repeatAngle = mConst->getPoleClimbInputRepeatAngle();
    if (input->isMoveDeepDownNoSnap()) {
        if (!input->isSameStickMove(_b8, repeatAngle)) {
            input->calcPoleMoveInput(&poleMoveInput);
            _b8 = input->getStickMoveRaw();
        }
    } else {
        poleMoveInput = {0.0f, 0.0f};
        _b8 = {0.0f, 0.0f};
    }

    sead::Vector3f up = {0.0f, 0.0f, 0.0f};
    al::calcUpDir(&up, mActor);
    sead::Vector3f side = {0.0f, 0.0f, 0.0f};
    al::calcSideDir(&side, mActor);
    const f32 inputDegree = mConst->getPoleClimbInputDegreeMove();
    if (calcPoleMoveDirection(input, up, side,
                              reinterpret_cast<IJudge*>(mJudgePreInputPoleClimbSwing), _b0, false,
                              poleMoveInput.x, poleMoveInput.y, inputDegree) == 2)
        _b4 = true;

    al::setNerve(this, &NrvPlayerStatePoleClimb.Start);
}

namespace {
// NON_MATCHING: target/current are 376/364 after replacing rejected scalarized dot products with sead vector helpers; semantics remain reconstructed. Next recover the target scalar/vector lifetime shape using validator-accepted sead operations.
s32 calcPoleMoveDirection(const PlayerInput* input, const sead::Vector3f& up,
                          const sead::Vector3f& side, const IJudge* judge, s32 moveDirection,
                          bool enableDown, f32 inputX, f32 inputY, f32 inputDegree) {
    const f32 inputYValue = side.y * inputX + up.y * inputY;
    const f32 inputXValue = side.x * inputX;
    const f32 inputZValue = side.z * inputX + up.z * inputY;
    const sead::Vector3f inputDir = {inputXValue + up.x * inputY, inputYValue, inputZValue};

    s32 result;
    if (al::isNearZero(inputDir, 0.001f)) {
        result = 2;
        if (moveDirection <= 0) {
            const bool isHoldDown = input->isHoldPoleClimbDown();
            result = 2;
            if (!isHoldDown)
                return rs::isJudge(judge) & 1;
        }
    } else {
        const f32 cosDegree = sead::Mathf::cos(sead::Mathf::deg2rad(inputDegree));
        const f32 upDot = up.dot(inputDir);
        result = 1;
        if (upDot <= cosDegree) {
            if (-upDot <= cosDegree) {
                result = 3;
                if (side.dot(inputDir) <= 0.0f)
                    return 4;
            } else if (enableDown && !input->isHoldPoleClimbDown()) {
                return 0;
            } else {
                return 2;
            }
        }
    }
    return result;
}
}  // namespace

// NON_MATCHING: exact-size 300; first difference is commutative FMUL operand order for scaled front.y at target 0x476E20; next try a source-natural per-component/scale helper shape that preserves front scalar loads without storing the scaled vector.
void PlayerStatePoleClimb::kill() {
    mHandLegAngle->handAngle.set(0.0f, 0.0f, 0.0f);
    mHandLegAngle->legAngle.set(0.0f, 0.0f, 0.0f);
    mHandLegAngle->blendRate = 1.0f;
    rs::validatePlayerGroundShadow(mModelHolder);

    if (isFormPoleClimb()) {
        al::LiveActor* actor = mActor;
        sead::Vector3f front = {0.0f, 0.0f, 0.0f};
        al::calcFrontDir(&front, actor);
        const f32 distance = mConst->getCollisionRadius() + 10.0f;
        sead::Vector3f offset;
        offset.setScale(front, distance);
        const sead::Vector3f& trans = al::getTrans(actor);
        al::setTrans(actor, trans - offset);
        rs::resetCollision(mCollision);
    }

    rs::tryClosePlayerClimbPoleTutorial(mActor);
    rs::tryClosePlayerClimbPoleTopTutorial(mActor);
    al::setNerve(this, &NrvPlayerStatePoleClimb.Start);
    al::NerveStateBase::kill();
}

bool PlayerStatePoleClimb::isFormPoleClimb() const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvPlayerStatePoleClimb.Jump) && al::isGreaterEqualStep(this, 1))
        return false;
    if (al::isNerve(this, &NrvPlayerStatePoleClimb.TopJump))
        return false;
    if (al::isNerve(this, &NrvPlayerStatePoleClimb.TopWait))
        return false;
    if (al::isNerve(this, &NrvPlayerStatePoleClimb.TopTurn))
        return false;
    if (al::isNerve(this, &NrvPlayerStatePoleClimb.TopStart))
        return al::isLessStep(this, mConst->getPoleTopStartFrame());
    return !al::isNerve(this, &NrvPlayerStatePoleClimb.TopEnd) || al::isGreaterEqualStep(this, 1);
}

bool PlayerStatePoleClimb::update() {
    rs::updateJudge(reinterpret_cast<IJudge*>(mJudgePreInputPoleClimbSwing));

    const PlayerInput* input = mInput;
    const f32 repeatAngle = mConst->getPoleClimbInputRepeatAngle();
    if (input->isMoveDeepDownNoSnap()) {
        if (!input->isSameStickMove(_a8, repeatAngle)) {
            input->calcPoleMoveInput(&_9c);
            _a8 = input->getStickMoveRaw();
        }
    } else {
        _9c = {0.0f, 0.0f};
        _a8 = {0.0f, 0.0f};
    }

    if (_b4) {
        sead::Vector2f poleMoveInput = {0.0f, 0.0f};
        const PlayerInput* oneShotInput = mInput;
        const f32 oneShotRepeatAngle = mConst->getPoleClimbInputRepeatAngle();
        if (!oneShotInput->isMoveDeepDownNoSnap()) {
            poleMoveInput = {0.0f, 0.0f};
            _b8 = {0.0f, 0.0f};
            _b4 = false;
        } else if (!oneShotInput->isSameStickMove(_b8, oneShotRepeatAngle)) {
            oneShotInput->calcPoleMoveInput(&poleMoveInput);
            _b8 = oneShotInput->getStickMoveRaw();
            _b4 = false;
        }
    }

    return al::NerveStateBase::update();
}

void PlayerStatePoleClimb::setup(const al::CollisionParts* collisionParts,
                                 const sead::Vector3f& position,
                                 const sead::Vector3f& front,
                                 const sead::Vector3f& up, f32 depth, f32 moveRate,
                                 const char* animationName) {
    updatePoleDepth(depth, moveRate);
    mCollisionSnap->setup(collisionParts, position, front, up);
    mMaterialCode = animationName;
}

void PlayerStatePoleClimb::updatePoleDepth(f32 depth, f32 moveRate) {
    const f32 minDepth = mConst->getPoleClimbCatchRangeMin();
    const f32 maxDepth = mConst->getPoleClimbCatchRangeMax();
    _88 = sead::Mathf::clamp(depth, minDepth, maxDepth);
    _8c = moveRate;

    const f32 centerDepth = mConst->getPoleClimbCatchRange();
    f32 jointAngle = 0.0f;
    if (!al::isNearZero(_88 - centerDepth, 0.001f)) {
        const f32 poleDepth = _88;
        if (!(poleDepth > centerDepth)) {
            const f32 rate = al::calcRate01(poleDepth, mConst->getPoleClimbJointRangeMin(),
                                            centerDepth);
            jointAngle = al::lerpValue(mConst->getPoleClimbJointAngleMin(), 0.0f, rate);
        } else {
            const f32 rate = al::calcRate01(poleDepth, centerDepth,
                                            mConst->getPoleClimbJointRangeMax());
            jointAngle = al::lerpValue(0.0f, mConst->getPoleClimbJointAngleMax(), rate);
        }
    }

    mHandLegAngle->handAngle = sead::Vector3f::ey * (jointAngle + _8c);
    mHandLegAngle->legAngle = sead::Vector3f::ey * (jointAngle + _8c);
}

bool PlayerStatePoleClimb::isAttachPole() const {
    if (!isDead()) {
        if (!al::isNerve(this, &NrvPlayerStatePoleClimb.Jump) ||
            !al::isGreaterEqualStep(this, 1)) {
            if (!al::isNerve(this, &NrvPlayerStatePoleClimb.TopJump))
                return true;
        }
    }
    return false;
}

bool PlayerStatePoleClimb::isPoleJump() const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvPlayerStatePoleClimb.Jump) && al::isGreaterEqualStep(this, 1))
        return true;
    return al::isNerve(this, &NrvPlayerStatePoleClimb.TopJump);
}

bool PlayerStatePoleClimb::isTriggerReaction() const {
    return !isDead() &&
           (al::isNerve(this, &NrvPlayerStatePoleClimb.Start) ||
            al::isNerve(this, &NrvPlayerStatePoleClimb.Up) ||
            al::isNerve(this, &NrvPlayerStatePoleClimb.Turn)) &&
           al::isStep(this, 1);
}

bool PlayerStatePoleClimb::isEnableTrample() const {
    if (isDead())
        return false;
    if (al::isNerve(this, &NrvPlayerStatePoleClimb.Jump) && al::isGreaterEqualStep(this, 1))
        return true;
    return al::isNerve(this, &NrvPlayerStatePoleClimb.TopJump);
}

bool PlayerStatePoleClimb::isForceFollowCap() const {
    return !isDead() && isFormPoleClimb() && !al::isNerve(this, &NrvPlayerStatePoleClimb.Wait);
}

const sead::Vector3f& PlayerStatePoleClimb::getPoleFront() const {
    return mCollisionSnap->getSnapFront();
}

al::HitSensor* PlayerStatePoleClimb::getPoleSensor() const {
    return mCollisionSnap->tryGetConnectedSensor();
}

void PlayerStatePoleClimb::updateLeavePoleTrans() const {
    if (!isFormPoleClimb())
        return;

    al::LiveActor* actor = mActor;
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    al::calcFrontDir(&front, actor);
    const sead::Vector3f& trans = al::getTrans(actor);
    const f32 distance = mConst->getCollisionRadius();
    al::setTrans(actor, trans - front * distance);
}

void PlayerStatePoleClimb::exeStart() {
    mCollisionSnap->followCollision();
    if (!mCollisionSnap->isSnapPartsValid()) {
        kill();
        return;
    }

    if (!tryStartClimbMove(0, nullptr) && !mAnimator->isAnimEnd())
        al::setNerve(this, &NrvPlayerStatePoleClimb.Wait);
}

// NON_MATCHING: target/current are 1712/1516; behavior and both search-call ABI argument sets are reconstructed, but current local lifetimes/branch scheduling are substantially more compact. Next align output/search temporary lifetimes against target assembly without attributes.
bool PlayerStatePoleClimb::tryStartClimbMove(s32 direction, s32* moveDirection) {
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    al::calcFrontDir(&front, mActor);
    sead::Vector3f side = {0.0f, 0.0f, 0.0f};
    al::calcSideDir(&side, mActor);
    sead::Vector3f up = {0.0f, 0.0f, 0.0f};
    al::calcUpDir(&up, mActor);

    s32 result = calcPoleMoveDirection(mInput, up, side,
                                       reinterpret_cast<IJudge*>(mJudgePreInputPoleClimbSwing),
                                       _b0, _b4, _9c.x, _9c.y,
                                       mConst->getPoleClimbInputDegreeMove());
    if (result == 2 && _b4)
        _b4 = false;

    if (_b0 >= 1 && result != 4)
        _b0 = 0;

    if (moveDirection)
        *moveDirection = result;
    if (result == 0)
        return false;
    if (direction != 0 && result == direction)
        return false;

    if (result == 3 || result == 4) {
        if (_a4) {
            _a5 = mInput->isPoleMoveInputReverseX();
            _a4 = false;
        }
        if (_a5) {
            result = result == 3 ? 4 : 3;
            if (moveDirection)
                *moveDirection = result;
        }
    }

    sead::Vector3f moveOffset = {0.0f, 0.0f, 0.0f};
    sead::Vector3f turnDir = {0.0f, 0.0f, 0.0f};
    f32 moveSpeed = 0.0f;
    s32 moveFrame = 0;
    bool useTurnSearch = false;
    bool setBlendRate = false;
    const al::Nerve* nerve = nullptr;

    if (result == 3 || result == 4) {
        const f32 turnDist = mConst->getPoleClimbTurnDist();
        if (result == 3) {
            moveOffset = side * turnDist;
            turnDir = -side;
        } else {
            moveOffset = -side * turnDist;
            turnDir = side;
        }
        moveFrame = mConst->getPoleClimbTurnFrame();
        nerve = &NrvPlayerStatePoleClimb.Turn;
        useTurnSearch = true;
        setBlendRate = true;
    } else if (result == 1) {
        moveSpeed = mConst->getPoleClimbUpSpeed();
        if (rs::isJudge(reinterpret_cast<IJudge*>(mJudgePreInputPoleClimbSwing))) {
            moveFrame = mConst->getPoleClimbUpFrameSwing();
            rs::resetJudge(reinterpret_cast<IJudge*>(mJudgePreInputPoleClimbSwing));
        } else if (mInput->isHoldPoleClimbFast()) {
            moveFrame = mConst->getPoleClimbUpFrameFast();
        } else {
            moveFrame = mConst->getPoleClimbUpFrame();
        }
        nerve = &NrvPlayerStatePoleClimb.Up;
        setBlendRate = true;
    } else {
        f32 downSpeed;
        if (_b0 < 1) {
            if (rs::isJudge(reinterpret_cast<IJudge*>(mJudgePreInputPoleClimbSwing))) {
                downSpeed = mConst->getPoleClimbDownSpeedSwing();
                moveFrame = mConst->getPoleClimbDownFrame();
                _b0 = mConst->getPoleClimbDownKeepTime();
            } else {
                downSpeed = mInput->isHoldPoleClimbFast() || mInput->isHoldPoleClimbDown()
                                ? mConst->getPoleClimbDownSpeedFast()
                                : mConst->getPoleClimbDownSpeed();
                moveFrame = mConst->getPoleClimbDownFrame();
            }
        } else {
            downSpeed = mConst->getPoleClimbDownSpeedSwing();
            moveFrame = mConst->getPoleClimbDownFrame();
        }
        moveSpeed = -downSpeed;
        nerve = &NrvPlayerStatePoleClimb.Down;
    }

    const al::CollisionParts* parts = nullptr;
    sead::Vector3f position = {0.0f, 0.0f, 0.0f};
    sead::Vector3f snapFront = {0.0f, 0.0f, 0.0f};
    sead::Vector3f snapUp = {0.0f, 0.0f, 0.0f};
    f32 depth = _88;
    f32 moveRate = _8c;
    const char* materialCode = nullptr;
    bool isTopStart = false;
    const sead::Vector3f targetPos = mCollisionSnap->mState._48 + moveOffset;

    bool found;
    if (useTurnSearch) {
        const f32 turnMargin = sead::Mathf::clampMin((_88 - mConst->getPoleClimbTurnDist()) * 0.5f,
                                                      0.0f);
        found = rs::findPoleClimbTurnPos(
            &parts, &position, &snapFront, &snapUp, &depth, &moveRate, &materialCode, mActor, front,
            up, targetPos, turnDir, 50.0f, mCollisionSnap->mState._48, _88 * 0.5f, _88,
            mConst->getPoleClimbCatchRangeMax(), turnMargin, 100.0f);
        if (!found) {
            _b0 = 0;
            return false;
        }
    } else {
        found = rs::findPoleClimbMovePos(
            &parts, &position, &snapFront, &snapUp, &depth, &moveRate, &materialCode, &isTopStart,
            mActor, front, up, targetPos, moveSpeed, mConst->getPoleClimbUpMargine(), 50.0f,
            mConst->getPoleClimbMoveWallDegree(), _88, mConst->getPoleClimbCatchRangeMax());
        if (isTopStart) {
            snapFront = al::isParallelDirection(front, snapUp, 0.01f) ? up : front;
            moveFrame = mConst->getPoleTopStartFrame();
            _90 = depth;
            depth = _88;
            if (!found) {
                _b0 = 0;
                return false;
            }
            nerve = &NrvPlayerStatePoleClimb.TopStart;
        } else if (!found) {
            _b0 = 0;
            return false;
        }
    }

    updatePoleDepth(depth, moveRate);
    mCollisionSnap->moveSnapPos(parts, position, snapFront, snapUp, moveFrame);
    mMaterialCode = materialCode;
    if (setBlendRate)
        mHandLegAngle->blendRate = 0.1f;
    al::setNerve(this, nerve);
    return true;
}

void PlayerStatePoleClimb::exeWait() {
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("PoleWait");
        mHandLegAngle->blendRate = 1.0f;
    }

    mCollisionSnap->followCollision();
    if (!mCollisionSnap->isSnapPartsValid()) {
        kill();
    } else if (rs::judgeAndResetReturnTrue(reinterpret_cast<IJudge*>(mJudgePreInputJump))) {
        al::setNerve(this, &NrvPlayerStatePoleClimb.Jump);
    } else {
        tryStartClimbMove(0, nullptr);
    }
}

void PlayerStatePoleClimb::exeUp() {
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("PoleClimb");
        _98 = mCollisionSnap->getMoveFrame();
        if (_98 == mConst->getPoleClimbUpFrame())
            _94 = 0;
        else if (_98 == mConst->getPoleClimbUpFrameFast())
            _94 = 1;
        else if (_98 == mConst->getPoleClimbUpFrameSwing())
            _94 = 2;

        const f32 animRate =
            mAnimator->getAnimFrameMax() / static_cast<f32>(_98 - 1) * 0.999f;
        mAnimator->setAnimRate(animRate);
        _a4 = true;
    }

    mCollisionSnap->updateMove();
    mCollisionSnap->followCollision();
    if (!mCollisionSnap->isSnapPartsValid()) {
        kill();
        return;
    }

    if (rs::judgeAndResetReturnTrue(reinterpret_cast<IJudge*>(mJudgePreInputJump))) {
        al::setNerve(this, &NrvPlayerStatePoleClimb.Jump);
        return;
    }

    if (mCollisionSnap->isMoveEnd()) {
        if (!tryStartClimbMove(0, nullptr))
            al::setNerve(this, &NrvPlayerStatePoleClimb.Wait);
        return;
    }

    if (tryStartClimbMove(1, nullptr) || _94 > 1)
        return;

    if (rs::isJudge(reinterpret_cast<const IJudge*>(mJudgePreInputPoleClimbSwing))) {
        rs::resetJudge(reinterpret_cast<IJudge*>(mJudgePreInputPoleClimbSwing));
        _94 = 2;
        changeUpMoveSpeed(al::getNerveStep(this) + 1, mConst->getPoleClimbUpFrameSwing());
        return;
    }

    if (_94 > 0 || !mInput->isHoldPoleClimbFast())
        return;
    _94 = 1;
    changeUpMoveSpeed(al::getNerveStep(this) + 1, mConst->getPoleClimbUpFrameFast());
}

void PlayerStatePoleClimb::changeUpMoveSpeed(s32 startFrame, s32 endFrame) {
    const f32 rate = static_cast<f32>(startFrame) / static_cast<f32>(_98);
    const f32 remainRate = 1.0f - rate;
    const f32 remainingFrame = static_cast<f32>(endFrame) * remainRate;
    const s32 moveFrame = sead::Mathf::ceil(remainingFrame);
    mCollisionSnap->restartMoveCurrentMtx(moveFrame);
    const f32 animFrame = rate * mAnimator->getAnimFrameMax();
    const f32 animRate =
        remainRate * mAnimator->getAnimFrameMax() / static_cast<f32>(moveFrame);
    mAnimator->setAnimFrame(animFrame);
    mAnimator->setAnimRate(animRate);
    _98 = moveFrame + startFrame;
}

void PlayerStatePoleClimb::exeDown() {
    if (al::isFirstStep(this)) {
        if (!mAnimator->isAnim("PoleFall"))
            mAnimator->startAnim("PoleFall");
        _a4 = true;
    }

    mCollisionSnap->updateMove();
    mCollisionSnap->followCollision();
    if (!mCollisionSnap->isSnapPartsValid()) {
        kill();
        return;
    }

    if (rs::judgeAndResetReturnTrue(reinterpret_cast<IJudge*>(mJudgePreInputJump))) {
        al::setNerve(this, &NrvPlayerStatePoleClimb.Jump);
        return;
    }

    if (al::isLessStep(this, mCollisionSnap->getMoveFrame() - 1)) {
        tryStartClimbMove(2, nullptr);
        return;
    }

    _b0 = al::converge(_b0, 0, 1);
    s32 moveDirection = 0;
    if (!tryStartClimbMove(0, &moveDirection)) {
        al::setNerve(this, &NrvPlayerStatePoleClimb.Wait);
        return;
    }

    if (moveDirection != 2 || !rs::isCollidedGround(mCollision))
        return;

    const sead::Vector3f& gravity = al::getGravity(mActor);
    sead::Vector3f up = {0.0f, 0.0f, 0.0f};
    al::calcUpDir(&up, mActor);
    al::normalize(&up);
    if (!al::isFloorPolygon(up, gravity))
        return;

    const sead::Vector3f& groundNormal = rs::getCollidedGroundNormal(mCollision);
    if (up.dot(groundNormal) < 0.70711f)
        return;

    mAnimator->startAnim("Fall");
    mAnimator->clearInterpolation();
    kill();
}

void PlayerStatePoleClimb::exeTopStart() {
    if (al::isFirstStep(this)) {
        mAnimator->startAnim("PoleHandStandStart");
        mHandLegAngle->legAngle.set(0.0f, 0.0f, 0.0f);
        rs::tryClosePlayerClimbPoleTutorial(mActor);
        rs::tryAppearPlayerClimbPoleTopTutorial(mActor);
    }

    mCollisionSnap->updateMove();
    mCollisionSnap->followCollision();
    if (!mCollisionSnap->isSnapPartsValid()) {
        kill();
        return;
    }

    rs::resetCollision(mCollision);
    if (rs::judgeAndResetReturnTrue(reinterpret_cast<IJudge*>(mJudgePreInputJump)))
        al::setNerve(this, &NrvPlayerStatePoleClimb.TopJump);
    else if (mAnimator->isAnimEnd())
        al::setNerve(this, &NrvPlayerStatePoleClimb.TopWait);
}

void PlayerStatePoleClimb::exeTopWait() {
    if (al::isFirstStep(this))
        mAnimator->startAnim("PoleHandStandWait");

    mCollisionSnap->updateMove();
    mCollisionSnap->followCollision();
    if (!mCollisionSnap->isSnapPartsValid()) {
        kill();
        return;
    }

    rs::resetCollision(mCollision);
    if (rs::judgeAndResetReturnTrue(reinterpret_cast<IJudge*>(mJudgePreInputJump)))
        al::setNerve(this, &NrvPlayerStatePoleClimb.TopJump);
    else
        tryTurnTopOrClimb();
}

// NON_MATCHING: target/current are 620/808 after the validator-required sead dot rewrite made the shared classifier inline again. Next recover a validator-accepted classifier shape near the target 376-byte cost before refining top-transition temporary lifetimes.
bool PlayerStatePoleClimb::tryTurnTopOrClimb() {
    sead::Vector3f up = {0.0f, 0.0f, 0.0f};
    al::calcUpDir(&up, mActor);
    sead::Vector3f side = {0.0f, 0.0f, 0.0f};
    al::calcSideDir(&side, mActor);

    const PlayerInput* input = mInput;
    const s32 direction =
        calcPoleMoveDirection(input, up, side,
                              reinterpret_cast<IJudge*>(mJudgePreInputPoleClimbSwing), _b0, false,
                              _9c.x, _9c.y, mConst->getPoleClimbInputDegreeMove());
    if (direction < 2)
        return false;

    if (direction == 2) {
        const al::CollisionParts* parts = nullptr;
        sead::Vector3f position = {0.0f, 0.0f, 0.0f};
        sead::Vector3f front = {0.0f, 0.0f, 0.0f};
        sead::Vector3f snapUp = {0.0f, 0.0f, 0.0f};
        const char* materialCode = nullptr;
        f32 moveRate = 0.0f;
        f32 depth = 0.0f;

        sead::Vector3f actorFront = {0.0f, 0.0f, 0.0f};
        al::calcFrontDir(&actorFront, mActor);
        const sead::Vector3f& trans = al::getTrans(mActor);
        const sead::Vector3f checkPos =
            trans - actorFront * (_90 * 0.5f) - up * mConst->getPoleTopEndUnderOffsetY();
        if (!rs::findPoleClimbFromTopPos(&parts, &position, &front, &snapUp, &depth, &moveRate,
                                         &materialCode, mActor, checkPos, 50.0f, _88,
                                         mConst->getPoleClimbCatchRangeMax()))
            return false;

        updatePoleDepth(depth, moveRate);
        mHandLegAngle->blendRate = 0.1f;
        mCollisionSnap->moveSnapPos(parts, position, front, snapUp, mConst->getPoleTopEndFrame());
        mMaterialCode = materialCode;
        al::setNerve(this, &NrvPlayerStatePoleClimb.TopEnd);
        return true;
    }

    f32 turnSpeed = mConst->getPoleTopTurnSpeed();
    if (direction != 3)
        turnSpeed = -turnSpeed;
    mCollisionSnap->turnSnapFrontAxisUp(turnSpeed);
    if (al::isNerve(this, &NrvPlayerStatePoleClimb.TopTurn))
        return true;
    al::setNerve(this, &NrvPlayerStatePoleClimb.TopTurn);
    return true;
}

void PlayerStatePoleClimb::exeTopTurn() {
    if (al::isFirstStep(this))
        mAnimator->startAnim("PoleHandStandTurn");

    mCollisionSnap->updateMove();
    mCollisionSnap->followCollision();
    if (!mCollisionSnap->isSnapPartsValid()) {
        kill();
        return;
    }

    rs::resetCollision(mCollision);
    if (rs::judgeAndResetReturnTrue(reinterpret_cast<IJudge*>(mJudgePreInputJump))) {
        al::setNerve(this, &NrvPlayerStatePoleClimb.TopJump);
    } else if (!tryTurnTopOrClimb()) {
        al::setNerve(this, &NrvPlayerStatePoleClimb.TopWait);
    }
}

void PlayerStatePoleClimb::exeTopEnd() {
    if (al::isFirstStep(this)) {
        _b4 = false;
        mAnimator->startAnim("PoleHandStandEnd");
        rs::tryClosePlayerClimbPoleTopTutorial(mActor);
        rs::tryAppearPlayerClimbPoleTutorial(mActor);
    }

    mCollisionSnap->updateMove();
    mCollisionSnap->followCollision();
    if (!mCollisionSnap->isSnapPartsValid()) {
        kill();
        return;
    }

    rs::resetCollision(mCollision);
    if (mAnimator->isAnimEnd()) {
        mHandLegAngle->blendRate = 1.0f;
        al::setNerve(this, &NrvPlayerStatePoleClimb.Wait);
    }
}

void PlayerStatePoleClimb::exeTurn() {
    if (al::isFirstStep(this) && !mAnimator->isAnim("PoleTurn"))
        mAnimator->startAnim("PoleTurn");

    mCollisionSnap->updateMove();
    mCollisionSnap->followCollision();
    if (!mCollisionSnap->isSnapPartsValid()) {
        kill();
        return;
    }

    if (rs::judgeAndResetReturnTrue(reinterpret_cast<IJudge*>(mJudgePreInputJump))) {
        sead::Vector3f leaveDir = {0.0f, 0.0f, 0.0f};
        if (mInput->isMove()) {
            const PlayerInput* input = mInput;
            const sead::Vector3f up = -al::getGravity(mActor);
            input->calcMoveDirection(&leaveDir, up);
        } else {
            al::calcFrontDir(&leaveDir, mActor);
            leaveDir.negate();
        }
        mCollisionSnap->forceMoveEndNearestLeaveDir(leaveDir);
        al::setNerve(this, &NrvPlayerStatePoleClimb.Jump);
        return;
    }

    const s32 turnFrame = mConst->getPoleClimbTurnFrame();
    const s32 turnStopFrame = mConst->getPoleClimbTurnStopFrame();
    if (!al::isLessStep(this, turnFrame + turnStopFrame - 1) &&
        !tryStartClimbMove(0, nullptr))
        al::setNerve(this, &NrvPlayerStatePoleClimb.Wait);
}

void PlayerStatePoleClimb::exeJump() {
    if (al::isFirstStep(this)) {
        mHandLegAngle->handAngle.set(0.0f, 0.0f, 0.0f);
        mHandLegAngle->legAngle.set(0.0f, 0.0f, 0.0f);
        mHandLegAngle->blendRate = 1.0f;
        rs::validatePlayerGroundShadow(mModelHolder);
        updateLeavePoleTrans();
        rs::tryClosePlayerClimbPoleTutorial(mActor);
    }

    if (al::updateNerveState(this))
        kill();
}

void PlayerStatePoleClimb::exeTopJump() {
    if (al::isFirstStep(this)) {
        mHandLegAngle->handAngle.set(0.0f, 0.0f, 0.0f);
        mHandLegAngle->legAngle.set(0.0f, 0.0f, 0.0f);
        mHandLegAngle->blendRate = 1.0f;
        rs::validatePlayerGroundShadow(mModelHolder);
        rs::tryClosePlayerClimbPoleTopTutorial(mActor);
    }

    if (al::updateNerveState(this))
        kill();
}

bool PlayerStatePoleClimb::followCollision() {
    mCollisionSnap->followCollision();
    return mCollisionSnap->isSnapPartsValid();
}

PlayerStatePoleClimb::~PlayerStatePoleClimb() = default;
