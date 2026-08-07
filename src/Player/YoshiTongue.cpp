#include "Player/YoshiTongue.h"

#include <algorithm>
#include <cstring>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/MatrixUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Shadow/ActorShadowUtil.h"

#include "Player/PlayerConst.h"
#include "Player/YoshiJudgeStartTongueClingFix.h"
#include "Player/YoshiTongueCollider.h"
#include "Player/YoshiTongueJointControlKeeper.h"
#include "Player/YoshiTongueTipConnector.h"
#include "Util/JudgeUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackInputFunction.h"
#include "Util/SensorMsgFunction.h"

namespace {
const sead::Vector3f cTongueJointTrans(0.0f, 10.0f, 10.0f);
const sead::Vector3f cTongueJointRotate(180.0f, 270.0f, 0.0f);

NERVE_IMPL(YoshiTongue, Stay);
NERVE_IMPL(YoshiTongue, Stretch);
NERVE_IMPL(YoshiTongue, Hit);
NERVE_IMPL(YoshiTongue, Shrink);
NERVE_IMPL(YoshiTongue, Return);
NERVE_IMPL(YoshiTongue, Eat);

class YoshiTongueNrvHide : public al::Nerve {
public:
    void execute(al::NerveKeeper* keeper) const override {
        keeper->getParent<YoshiTongue>()->makeActorDead();
    }
};

NERVE_IMPL(YoshiTongue, ClingWall);
NERVE_IMPL(YoshiTongue, ClingGround);
NERVES_MAKE_STRUCT(YoshiTongue, Stay, Stretch, Hit, Shrink, Return, Eat, Hide, ClingWall,
                   ClingGround);
}  // namespace

void YoshiTongue::calcAnim() {
    al::LiveActor::calcAnim();
    sead::BoundBox3f boundingBox;
    mJointControlKeeper->calcTongueBoundingBox(&boundingBox);
    al::setDepthShadowMapBoundingBox(this, boundingBox.getMin(), boundingBox.getMax(), "Ground");
}

void YoshiTongue::startShrink() {
    al::setNerve(this, &NrvYoshiTongue.Shrink);
}

void YoshiTongue::endShrink() {
    al::setNerve(this, &NrvYoshiTongue.Return);
}

void YoshiTongue::eatFinish() {
    al::HitSensor* attackSensor = al::getHitSensor(this, "Attack");
    const s32 bindCount = mEatBindInfo.size();
    for (s32 i = 0; i < bindCount; i++) {
        al::HitSensor* sensor = mEatBindInfo.unsafeAt(i)->sensor;
        if (rs::sendMsgYoshiTongueEatBindFinish(sensor, attackSensor))
            al::setScaleAll(al::getSensorHost(sensor), mEatBindInfo.unsafeAt(i)->scale);
    }

    al::startHitReaction(mHost, "舌を使って食べる");
    mEatBindInfo.clear();
    al::setNerve(this, &NrvYoshiTongue.Stay);
    makeActorDead();
}

void YoshiTongue::endHack() {
    al::HitSensor* attackSensor = al::getHitSensor(this, "Attack");
    const s32 bindCount = mEatBindInfo.size();
    for (s32 i = 0; i < bindCount; i++) {
        al::HitSensor* sensor = mEatBindInfo.unsafeAt(i)->sensor;
        al::setScaleAll(al::getSensorHost(sensor), mEatBindInfo.unsafeAt(i)->scale);
        rs::sendMsgYoshiTongueEatBindCancel(sensor, attackSensor);
    }

    mEatBindInfo.clear();
    if (al::isDead(this) || al::isNerve(this, &NrvYoshiTongue.Stay) ||
        al::isNerve(this, &NrvYoshiTongue.Return))
        return;
    al::setNerve(this, &NrvYoshiTongue.Stay);
}

bool YoshiTongue::isEnableStartAttack() const {
    if (al::isDead(this))
        return true;
    return al::isNerve(this, &NrvYoshiTongue.Stay);
}

bool YoshiTongue::isEnableLookAtTip() const {
    if (al::isDead(this))
        return false;
    if (al::isNerve(this, &NrvYoshiTongue.Stay))
        return false;
    if (al::isNerve(this, &NrvYoshiTongue.Return))
        return false;
    if (al::isNerve(this, &NrvYoshiTongue.Eat))
        return false;
    if (al::isNerve(this, &NrvYoshiTongue.Stretch))
        return al::isGreaterEqualStep(this, 1);
    return true;
}

bool YoshiTongue::isEnableShrinkStart() const {
    if (al::isDead(this))
        return false;
    if (!al::isNerve(this, &NrvYoshiTongue.ClingWall) &&
        !al::isNerve(this, &NrvYoshiTongue.ClingGround))
        return false;
    if (!al::isGreaterEqualStep(this, 1))
        return false;
    return rs::isJudge(mJudgeStartClingFix);
}

bool YoshiTongue::isEnableEatFinish() const {
    if (al::isDead(this))
        return false;
    if (!al::isNerve(this, &NrvYoshiTongue.Eat))
        return false;
    return al::isGreaterStep(this, *mParam->eatStep);
}

bool YoshiTongue::isExistEatBind() const {
    return !mEatBindInfo.isEmpty();
}

bool YoshiTongue::isShrinkMove() const {
    if (al::isDead(this))
        return false;
    return al::isNerve(this, &NrvYoshiTongue.Shrink);
}

bool YoshiTongue::isConnectWall() const {
    if (al::isDead(this))
        return false;
    if (!al::isNerve(this, &NrvYoshiTongue.ClingWall) &&
        !al::isNerve(this, &NrvYoshiTongue.ClingGround) &&
        !al::isNerve(this, &NrvYoshiTongue.Shrink))
        return false;
    return !mTipConnector->isGroundAttached();
}

bool YoshiTongue::isConnectGround() const {
    if (al::isDead(this))
        return false;
    if (!al::isNerve(this, &NrvYoshiTongue.ClingWall) &&
        !al::isNerve(this, &NrvYoshiTongue.ClingGround) &&
        !al::isNerve(this, &NrvYoshiTongue.Shrink))
        return false;
    return mTipConnector->isGroundAttached();
}

void YoshiTongue::calcYoshiFaceDir(sead::Vector3f* faceDir) const {
    sead::Matrix34f headMtx = sead::Matrix34f::ident;
    al::makeMtxFollowTarget(&headMtx, *al::getJointMtxPtr(mModelActor, "Head"),
                            cTongueJointTrans, cTongueJointRotate);
    headMtx.getBase(*faceDir, 2);
}

void YoshiTongue::calcTongueTipPos(sead::Vector3f* tipPos) const {
    tipPos->z = mTongueTipPos.z;
    std::memcpy(tipPos, &mTongueTipPos, sizeof(sead::Vector2f));
}

bool YoshiTongue::tryCalcTonguePullForce(f32* force, sead::Vector3f* direction) const {
    sead::Vector3f distance(0.0f, 0.0f, 0.0f);
    if (!tryCalcTonguePullDistance(&distance))
        return false;

    f32 distanceLength = 0.0f;
    sead::Vector3f pullDirection(0.0f, 0.0f, 0.0f);
    if (al::separateScalarAndDirection(&distanceLength, &pullDirection, distance))
        return false;

    const f32 velocity = std::max(-pullDirection.dot(al::getVelocity(mHost)), 0.0f);
    if (distanceLength + velocity < 150.0f)
        return false;

    const f32 pullForce = distanceLength - mShrinkRestRange;
    if (!al::isNearZeroOrGreater(velocity + pullForce, 0.001f))
        return false;

    *force = pullForce;
    direction->z = pullDirection.z;
    std::memcpy(direction, &pullDirection, sizeof(sead::Vector2f));
    return true;
}

bool YoshiTongue::tryCalcTonguePullDistance(sead::Vector3f* distance) const {
    if (!al::isNerve(this, &NrvYoshiTongue.ClingWall) &&
        !al::isNerve(this, &NrvYoshiTongue.ClingGround) &&
        !al::isNerve(this, &NrvYoshiTongue.Shrink))
        return false;

    sead::Vector3f connectPos(0.0f, 0.0f, 0.0f);
    sead::Vector3f front(0.0f, 0.0f, 0.0f);
    sead::Vector3f up(0.0f, 0.0f, 0.0f);
    if (!mTipConnector->tryCalcConnect(&front, &up, &connectPos))
        connectPos = mTongueTipPos;

    sead::Matrix34f headMtx = sead::Matrix34f::ident;
    al::makeMtxFollowTarget(&headMtx, *al::getJointMtxPtr(mModelActor, "Head"),
                            cTongueJointTrans, cTongueJointRotate);
    const sead::Vector3f headPos = headMtx.getTranslation();
    sead::Vector3CalcCommon<f32>::sub(*distance, mTongueTipPos, headPos);
    return true;
}

bool YoshiTongue::tryCalcTongueConnect(const al::CollisionParts** collisionParts,
                                       sead::Vector3f* connectPos,
                                       sead::Vector3f* connectNormal,
                                       sead::Vector3f* direction,
                                       sead::Vector3f* tipPos) const {
    if (!al::isNerve(this, &NrvYoshiTongue.ClingWall) &&
        !al::isNerve(this, &NrvYoshiTongue.ClingGround) &&
        !al::isNerve(this, &NrvYoshiTongue.Shrink))
        return false;
    if (!mTipConnector->tryCalcConnect(connectNormal, direction, connectPos))
        return false;

    *collisionParts = mTipConnector->getCollisionParts();
    tipPos->z = direction->z;
    std::memcpy(tipPos, direction, sizeof(sead::Vector2f));
    return true;
}

f32 YoshiTongue::getShrinkRestRange() const {
    return std::max(mShrinkRestRange - 150.0f, 0.0f);
}

void YoshiTongue::adjustShrinkRestRange(f32 range) {
    const f32 adjustedRange = range + 150.0f;
    f32 clampedRange;
    if (adjustedRange < 150.0f) {
        clampedRange = 150.0f;
    } else {
        clampedRange = adjustedRange;
        if (adjustedRange > 1000.0f)
            clampedRange = 1000.0f;
    }
    mShrinkRestRange = clampedRange;
}

f32 YoshiTongue::getTongueParamSpeed() const {
    const bool isHack = mIsHack;
    const f32 speed = *mParam->speed;
    if (!isHack)
        return speed;
    return speed + speed;
}

f32 YoshiTongue::getTongueParamRange() const {
    return *mParam->range;
}

// NON_MATCHING: current body is 0x208 bytes versus target 0x1FC; an explicit early false return preserves the same output. Next test source-level lifetime/order changes around isCancel and the connector branch.
bool YoshiTongue::reactionCollideWall() {
    if (!rs::isCollidedWall(mTongueCollider))
        return false;

    mVelocity.set(0.0f, 0.0f, 0.0f);
    bool isCancel = rs::isActionCodeNoTongueClingWall(mTongueCollider);
    if (!isCancel) {
        const sead::Vector3f& wallPos = rs::getCollidedWallPos(mTongueCollider);
        const sead::Vector3f& gravity = al::getGravity(this);
        sead::Vector3f wallDistance = wallPos - al::getTrans(this);
        al::verticalizeVec(&wallDistance, gravity, wallDistance);
        isCancel = rs::isCollidedGround(mCollision) && wallDistance.length() < 150.0f;
    }

    if (isCancel) {
        al::startHitReaction(this, "壁接触による伸びキャンセル");
        if (isExistEatBind())
            al::setNerve(this, &NrvYoshiTongue.Eat);
        else
            al::setNerve(this, &NrvYoshiTongue.Return);
    } else {
        mTipConnector->tryCalcConnect(&mTongueDir, &mUpDir, &mTongueTipPos);
        if ((rs::isHoldHackAction(*mPlayerHack) || mIsHack) && !isExistEatBind()) {
            al::startHitReaction(this, "壁接触によるくっつき");
            mJudgeStartClingFix->setCheckWall();
            rs::updateJudgeAndResult(mJudgeStartClingFix);
            al::setNerve(this, &NrvYoshiTongue.ClingWall);
        } else if (al::isNerve(this, &NrvYoshiTongue.Hit)) {
            al::startHitReaction(this, "壁接触による伸びキャンセル");
            if (isExistEatBind())
                al::setNerve(this, &NrvYoshiTongue.Eat);
            else
                al::setNerve(this, &NrvYoshiTongue.Return);
        } else {
            al::startHitReaction(this, "壁接触");
            al::setNerve(this, &NrvYoshiTongue.Hit);
        }
    }

    return true;
}

bool YoshiTongue::reactionCollideGround() {
    if (!rs::isCollidedGround(mTongueCollider))
        return false;

    if (!mIsStayClingGround) {
        if (!rs::isHoldHackAction(*mPlayerHack) && !mIsHack)
            return false;

        const IUsePlayerCollision* collision = mTongueCollider;
        const f32 angle = mPlayerConst->getStandAngleMin();
        if (!rs::isCollidedGroundOverAngle(this, collision, angle))
            return false;
    }

    if (!mTipConnector->tryCalcConnect(&mTongueDir, &mUpDir, &mTongueTipPos))
        return false;

    if (rs::isActionCodeNoTongueClingGround(mTongueCollider)) {
        al::startHitReaction(this, "床接触による伸びキャンセル");
        if (isExistEatBind())
            al::setNerve(this, &NrvYoshiTongue.Eat);
        else
            al::setNerve(this, &NrvYoshiTongue.Return);
        return true;
    }

    if (isExistEatBind())
        return false;

    mVelocity.set(0.0f, 0.0f, 0.0f);
    al::startHitReaction(this, "床接触によるくっつき");
    mJudgeStartClingFix->setCheckGround();
    rs::updateJudgeAndResult(mJudgeStartClingFix);
    al::setNerve(this, &NrvYoshiTongue.ClingGround);
    return true;
}

void YoshiTongue::returnOrEatHide() {
    if (isExistEatBind())
        al::setNerve(this, &NrvYoshiTongue.Eat);
    else
        al::setNerve(this, &NrvYoshiTongue.Return);
}

bool YoshiTongue::isEnableStayClingGround() const {
    if (mIsStayClingGround)
        return true;
    if (!rs::isHoldHackAction(*mPlayerHack) && !mIsHack)
        return false;

    const IUsePlayerCollision* collision = mTongueCollider;
    const f32 angle = mPlayerConst->getStandAngleMin();
    return rs::isCollidedGroundOverAngle(this, collision, angle);
}

void YoshiTongue::exeHit() {}

void YoshiTongue::exeHide() {
    makeActorDead();
}

bool YoshiTongue::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                             al::HitSensor* self) {
    return false;
}
