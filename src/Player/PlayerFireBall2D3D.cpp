#include "Player/PlayerFireBall2D3D.h"

#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseKeeper.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/MatrixUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Shadow/ActorShadowUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"

#include "Util/AreaUtil.h"
#include "Util/CollisionUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(PlayerFireBall2D3D, Move)
NERVE_IMPL(PlayerFireBall2D3D, Dead)
NERVES_MAKE_STRUCT(PlayerFireBall2D3D, Move, Dead)
}  // namespace

// NON_MATCHING: the 0x138-byte layout and constructor initialization are recovered; remaining differences are base/member initialization register allocation.
PlayerFireBall2D3D::PlayerFireBall2D3D(const al::LiveActor* player)
    : al::LiveActor("プレイヤーファイアボール2D3D"), mPlayer(player), mIsBound(false),
      mIsIn2D(false), mIsShoot2D(false), _113(0), mAreaUp(0.0f, 0.0f, 1.0f),
      mAreaLockDir(sead::Vector3f::zero), _12c{}, mCollider2D3D(nullptr) {}

void PlayerFireBall2D3D::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, "PlayerFireBall2D3D", nullptr);
    mCollider2D3D =
        rs::createCollider2D3D(this, al::getColliderRadius(this), al::getColliderOffsetY(this),
                               sead::Vector3f::ex, 5.0f);
    al::initNerve(this, &NrvPlayerFireBall2D3D.Move, 0);
    makeActorDead();
}


// NON_MATCHING: complete 2D/3D launch setup is recovered from the corpus; remaining differences are vector temporary and call ordering.
void PlayerFireBall2D3D::shoot(bool is2D) {
    mIsShoot2D = is2D;
    if (is2D)
        al::startAction(this, "Change2D");

    sead::Vector3f up;
    al::calcUpDir(&up, mPlayer);
    sead::Vector3f front;
    al::calcFrontDir(&front, mPlayer);
    sead::Vector3f position = al::getTrans(mPlayer) + up * 100.0f + front * 50.0f;

    al::calcFrontDir(&front, mPlayer);
    sead::Vector3f poseUp = -al::getGravity(mPlayer);
    if (al::isParallelDirection(front, poseUp, 0.01f))
        al::calcUpDir(&poseUp, mPlayer);

    sead::Vector3f arrow = front * 80.0f;
    sead::Vector3f arrowStart = position - arrow;
    sead::Vector3f hitPos;
    sead::Vector3f hitNormal;
    if (alCollisionUtil::getHitPosAndNormalOnArrow(this, &hitPos, &hitNormal, arrowStart, arrow,
                                                    nullptr, nullptr))
        position = hitPos + hitNormal * al::getSensorRadius(this);

    sead::Matrix34f poseMtx;
    al::makeMtxUpFrontPos(&poseMtx, poseUp, front, position);
    al::updatePoseMtx(this, &poseMtx);

    al::setVelocity(this, front * 22.5f - poseUp * 15.0f);
    al::setGravity(this, al::getGravity(mPlayer));
    if (rs::isIn2DArea(mPlayer, &mAreaUp, &mAreaLockDir))
        al::resetPosition(this, position);

    mIsBound = false;
    rs::resetCollider(mCollider2D3D);
    updateCollider();
    makeActorAlive();
    al::setNerve(this, &NrvPlayerFireBall2D3D.Move);
}

// NON_MATCHING: complete movement, collision, lifetime, and transition behavior is recovered from the corpus; remaining differences are expression/register shape.
void PlayerFireBall2D3D::exeMove() {
    al::AreaObj* area = rs::tryFind2DAreaObj(this, &mAreaUp, &mAreaLockDir);
    if (area) {
        f32 gravityPower = 0.0f;
        if (!mIsIn2D) {
            al::tryStartActionIfNotPlaying(this, "Change2D");
            mIsIn2D = true;
            if (!mIsBound) {
                const sead::Vector3f& gravity = al::getGravity(this);
                gravityPower = sead::Mathf::abs(mAreaUp.dot(gravity)) *
                               gravity.dot(al::getVelocity(this));
            }
        }
        if (!al::isHideShadowMask(this))
            al::hideShadowMask(this);
        al::addVelocityToDirection(this, al::getGravity(this), gravityPower);
    } else {
        if (al::isHideShadowMask(this))
            al::showShadowMask(this);
        mIsIn2D = false;
        al::setGravity(this, -sead::Vector3f::ey);
        if (!mIsShoot2D)
            al::tryStartActionIfNotPlaying(this, "Change3D");
    }

    const sead::Vector3f& gravity = al::getGravity(this);
    const sead::Vector3f& velocity = al::getVelocity(this);
    if (rs::isCollided(mCollider2D3D))
        mIsBound = true;

    sead::Vector3f horizontalVelocity;
    al::verticalizeVec(&horizontalVelocity, gravity, velocity);
    horizontalVelocity *= 0.997f;
    if (al::isNearZero(horizontalVelocity, 0.001f)) {
        if (mIsIn2D)
            rs::calc2DAreaFreeDir(&horizontalVelocity, area, al::getTrans(this));
        else
            al::calcFrontDir(&horizontalVelocity, this);
    }

    const f32 gravitySpeed = gravity.dot(velocity);
    if (horizontalVelocity.length() < 14.0f) {
        const f32 horizontalSpeed = horizontalVelocity.length();
        if (horizontalSpeed > 0.0f)
            horizontalVelocity *= 14.0f / horizontalSpeed;
    }
    al::setVelocity(this, horizontalVelocity + gravity * gravitySpeed);

    bool skipBoundGravity = false;
    if (rs::isCollidedGround(mCollider2D3D) && velocity.dot(gravity) > 0.0f) {
        const sead::Vector3f& groundNormal = rs::getCollidedGroundNormal(mCollider2D3D);
        if ((-gravity).dot(groundNormal) > 0.5f) {
            sead::Vector3f* velocityPtr = al::getVelocityPtr(this);
            al::verticalizeVec(velocityPtr, gravity, *velocityPtr);
            al::addVelocityToGravity(this, -15.0f);
            al::startHitReactionHitEffect(this, "床バウンド",
                                          rs::getCollidedGroundPos(mCollider2D3D));
            skipBoundGravity = true;
        } else {
            boundWall(groundNormal, rs::getCollidedGroundPos(mCollider2D3D));
        }
    } else if (rs::isCollidedWallVelocity(this, mCollider2D3D)) {
        boundWall(rs::getCollidedWallNormal(mCollider2D3D),
                  rs::getCollidedWallPos(mCollider2D3D));
    } else if (rs::isCollidedCeilingVelocity(this, mCollider2D3D) &&
               velocity.dot(gravity) < 0.0f) {
        sead::Vector3f* velocityPtr = al::getVelocityPtr(this);
        al::verticalizeVec(velocityPtr, gravity, *velocityPtr);
    }

    if (!skipBoundGravity)
        applyGravity();
    turn(al::getVelocity(this));
    if (!al::isLessStep(this, 180))
        kill();
}

// NON_MATCHING: behavior and function size match; one commutative floating-point multiply has reversed operands, so the next hypothesis is the original scaled-vector expression form.
void PlayerFireBall2D3D::boundWall(const sead::Vector3f& normal,
                                    const sead::Vector3f& position) {
    sead::Vector3f wallNormal = normal;
    al::verticalizeVec(&wallNormal, al::getGravity(this), wallNormal);
    if (!al::tryNormalizeOrZero(&wallNormal))
        return;

    f32 reflectPower = wallNormal.dot(al::getVelocity(this)) * 2.0f;
    if (reflectPower >= 0.0f)
        return;

    *al::getVelocityPtr(this) -= wallNormal * reflectPower;
    al::startHitReactionHitEffect(this, "壁バウンド", position);
}

void PlayerFireBall2D3D::applyGravity() {
    if (mIsBound)
        al::addVelocityToGravity(this, 2.0f);
}


void PlayerFireBall2D3D::turn(const sead::Vector3f& velocity) {
    sead::Vector3f front;
    if (mIsIn2D) {
        al::verticalizeVec(&front, mAreaUp, velocity);
        if (!al::tryNormalizeOrZero(&front, velocity))
            return;

        sead::Vector3f up = al::getGravity(this);
        up.negate();
        sead::Quatf currentQuat;
        al::calcQuat(&currentQuat, this);
        sead::Quatf targetQuat;
        if (al::isParallelDirection(front, up, 0.01f)) {
            sead::Vector3f currentFront;
            al::calcFrontDir(&currentFront, this);
            al::makeQuatRotationRate(&targetQuat, currentFront, front, 1.0f);
            targetQuat = targetQuat * currentQuat;
        } else {
            al::makeQuatFrontUp(&targetQuat, front, up);
        }
        al::updatePoseQuat(this, targetQuat);
        return;
    }

    if (!al::tryNormalizeOrZero(&front, velocity))
        return;

    sead::Vector3f up = al::getGravity(this);
    up.negate();
    sead::Quatf currentQuat;
    al::calcQuat(&currentQuat, this);
    sead::Quatf targetQuat;
    if (al::isParallelDirection(front, up, 0.01f)) {
        sead::Vector3f currentFront;
        al::calcFrontDir(&currentFront, this);
        al::makeQuatRotationRate(&targetQuat, currentFront, front, 0.5f);
        targetQuat = targetQuat * currentQuat;
    } else {
        al::makeQuatFrontUp(&targetQuat, front, up);
        al::slerpQuat(&targetQuat, currentQuat, targetQuat, 0.5f);
    }
    al::updatePoseQuat(this, targetQuat);
}

void PlayerFireBall2D3D::exeDead() {
    kill();
}

void PlayerFireBall2D3D::updateCollider() {
    if (mIsIn2D || mIsShoot2D)
        rs::onCollide2D(mCollider2D3D);
    else
        rs::onCollide3D(mCollider2D3D);
    rs::updateCollider2D3D(this, mCollider2D3D);
}

void PlayerFireBall2D3D::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (!al::isNerve(this, &NrvPlayerFireBall2D3D.Move) || !al::isGreaterEqualStep(this, 1))
        return;

    bool isAttackSuccess = false;
    if (mIsIn2D)
        isAttackSuccess = rs::sendMsgPlayerFireBallAttack2D(other, self);
    else
        isAttackSuccess = rs::sendMsgPlayerFireBallAttack3D(other, self);

    if (!isAttackSuccess)
        isAttackSuccess = al::sendMsgPlayerFireBallAttack(other, self);
    if (isAttackSuccess)
        al::setNerve(this, &NrvPlayerFireBall2D3D.Dead);
}

bool PlayerFireBall2D3D::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                                      al::HitSensor* self) {
    return false;
}
