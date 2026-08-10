#include "Player/YoshiTongue.h"

#include <algorithm>
#include <prim/seadMemUtil.h>

#include "Library/Area/AreaObjUtil.h"
#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/Collision/CollisionPartsTriangle.h"
#include "Library/Collision/PartsInterpolator.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorParamHolder.h"
#include "Library/LiveActor/ActorParamHolderUtil.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/MatrixUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Shadow/ActorShadowUtil.h"

#include "Player/PlayerConst.h"
#include "Player/PlayerEyeSensorHitHolder.h"
#include "Player/PlayerWallActionHistory.h"
#include "Player/YoshiJudgeStartTongueClingFix.h"
#include "Player/YoshiTongueCollider.h"
#include "Player/YoshiTongueJointControlKeeper.h"
#include "Player/YoshiTongueTipConnector.h"
#include "Util/JudgeUtil.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackInputFunction.h"
#include "Util/SensorMsgFunction.h"
#include "Util/YoshiUtil.h"

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

bool tryResetTongueCollision(sead::Vector3f* position, YoshiTongueCollider* collider,
                             const al::LiveActor* actor);
void calcTongueDirection(sead::Vector3f* tongueDir, sead::Vector3f* upDir,
                         al::LiveActor* actor, const sead::Vector3f& direction);
void attachTongueTipCollision(YoshiTongueTipConnector* connector, const al::LiveActor* actor,
                              const al::CollisionParts* collisionParts,
                              const sead::Vector3f& normal, const sead::Vector3f& position,
                              const sead::Vector3f& tongueDir);
}  // namespace

YoshiTongue::YoshiTongue(const al::LiveActor* host, const al::LiveActor* modelActor,
                         const IUsePlayerCollision* collision,
                         const PlayerWallActionHistory* wallActionHistory,
                         const PlayerEyeSensorHitHolder* eyeSensorHitHolder,
                         const PlayerConst* playerConst, IUsePlayerHack** playerHack,
                         const char* actorName)
    : al::LiveActor(actorName),
      mHost(host),
      mModelActor(modelActor),
      mCollision(collision),
      mWallActionHistory(wallActionHistory),
      mEyeSensorHitHolder(eyeSensorHitHolder),
      mPlayerConst(playerConst),
      mPlayerHack(playerHack),
      mIsHack(false),
      mStartPos(0.0f, 0.0f, 0.0f),
      mTongueDir(0.0f, 0.0f, 0.0f),
      mUpDir(0.0f, 0.0f, 0.0f),
      mTongueTipPos(0.0f, 0.0f, 0.0f),
      mVelocity(0.0f, 0.0f, 0.0f),
      _1e0(0),
      _1e4(0.0f),
      mReturnOffset(0.0f, 0.0f, 0.0f),
      mShrinkRestRange(0.0f),
      mAttackSensorPos(0.0f, 0.0f, 0.0f),
      mIsStayClingGround(false),
      _214(0) {
    mCollisionBuffer.tryAllocBuffer(4, nullptr, 8);

    mEatBindInfo.allocBuffer(16, nullptr);
    mEatBindInfoBuffer.tryAllocBuffer(16, nullptr, 8);
    for (s32 i = 0; i < 16; i++)
        mEatBindInfoBuffer[i] = new YoshiTongueEatBindInfo;
}

void YoshiTongue::init(const al::ActorInitInfo& info) {
    al::initChildActorWithArchiveNameNoPlacementInfo(this, info, "YoshiTongue", nullptr);

    YoshiTongueParam* param = new YoshiTongueParam;
    param->speed = al::findActorParamF32(this, "最高速度");
    param->stretchStep = al::findActorParamS32(this, "ブレーキ時間");
    param->range = al::findActorParamF32(this, "到達距離");
    param->clingWallStep = al::findActorParamS32(this, "端点停止時間");
    param->eatStep = al::findActorParamS32(this, "戻りフレーム");
    param->pullForce = al::findActorParamF32(this, "戻り強さ");
    param->pullSpeed = al::findActorParamF32(this, "戻り最高速度");
    mParam = param;

    mTongueCollider = new YoshiTongueCollider(this);
    mJointControlKeeper = new YoshiTongueJointControlKeeper(this, mModelActor);
    mTipConnector = new YoshiTongueTipConnector(this);
    mJudgeStartClingFix =
        new YoshiJudgeStartTongueClingFix(this, mTongueCollider, mWallActionHistory);

    al::setHitSensorPosPtr(this, "AttackLine", &mAttackSensorPos);
    al::setSensorRadius(this, "AttackLine", 0.0f);
    al::initNerve(this, &NrvYoshiTongue.Stay, 0);
    makeActorDead();
}

void YoshiTongue::updateCollider() {
    if (al::isNoCollide(this)) {
        mTongueTipPos += mVelocity;
        mTongueCollider->resetCollision(mTongueTipPos);
    } else if (al::isNerve(this, &NrvYoshiTongue.ClingWall) ||
               al::isNerve(this, &NrvYoshiTongue.ClingGround) ||
               al::isNerve(this, &NrvYoshiTongue.Shrink)) {
        mTongueCollider->collide(mTongueTipPos, mVelocity);
    } else {
        mTongueTipPos += mTongueCollider->collide(mTongueTipPos, mVelocity);
    }

    if (al::isNerve(this, &NrvYoshiTongue.Stretch) ||
        al::isNerve(this, &NrvYoshiTongue.Hit)) {
        if (rs::isCollidedWall(mTongueCollider)) {
            mTongueTipPos.set(rs::getCollidedWallPos(mTongueCollider));
            YoshiTongueTipConnector* connector = mTipConnector;
            const al::CollisionParts* collisionParts =
                rs::getCollidedWallCollisionParts(mTongueCollider);
            const sead::Vector3f& normal = rs::getCollidedWallNormal(mTongueCollider);
            attachTongueTipCollision(connector, this, collisionParts, normal, mTongueTipPos,
                                     mTongueDir);
        } else if (rs::isCollidedGround(mTongueCollider)) {
            YoshiTongueTipConnector* connector = mTipConnector;
            const al::CollisionParts* collisionParts =
                rs::getCollidedGroundCollisionParts(mTongueCollider);
            const sead::Vector3f& normal = rs::getCollidedGroundNormal(mTongueCollider);
            const sead::Vector3f& position = rs::getCollidedGroundPos(mTongueCollider);
            sead::Vector3f direction(0.0f, 0.0f, 0.0f);
            al::alongVectorNormalH(&direction, mTongueDir, mUpDir, normal);
            if (al::tryNormalizeOrZero(&direction))
                connector->attachCollision(collisionParts, direction, normal, position, normal,
                                           al::getGravity(this));
        }
    }

    mJointControlKeeper->update(mTongueDir, mUpDir, mTongueTipPos);

    sead::Vector3f offset = mTongueTipPos - al::getTrans(this);
    mAttackSensorPos.setScaleAdd(0.5f, offset, al::getTrans(this));
    const f32 tongueLength = offset.length();
    al::setSensorRadius(this, "AttackLine", tongueLength * 0.5f);
    if (!al::tryNormalizeOrZero(&mFaceDir, offset)) {
        al::calcFrontDir(&mFaceDir, this);
        al::normalize(&mFaceDir);
    }

    if (!al::isNerve(this, &NrvYoshiTongue.Stay)) {
        sead::Vector3f side = mFaceDir.cross(al::getGravity(this));
        if (al::tryNormalizeOrZero(&side)) {
            const sead::Vector3f sideStart = al::getTrans(this) - side * 80.0f;
            const sead::Vector3f sideEnd = sideStart + side * 160.0f;
            const sead::Vector3f along = mFaceDir * tongueLength;
            const sead::Vector3f quadEnd = sideEnd + along;
            const sead::Vector3f quadStart = sideStart + along;
            al::tryAddQuadRipple(this, quadStart, quadEnd, sideEnd, sideStart, 0.02f);
        }
    }

    updateEatBindActor();
}

namespace {
void attachTongueTipCollision(YoshiTongueTipConnector* connector, const al::LiveActor* actor,
                              const al::CollisionParts* collisionParts,
                              const sead::Vector3f& normal, const sead::Vector3f& position,
                              const sead::Vector3f& tongueDir) {
    sead::Vector3f front(0.0f, 0.0f, 0.0f);
    front.setCross(normal, al::getGravity(actor));
    al::normalize(&front);

    const f32 dot = front.dot(tongueDir);
    const f32 absDot = sead::Mathf::abs(dot);
    if (absDot < 0.86603f) {
        sead::Vector3f rotateFront = normal.cross(front);
        al::normalize(&rotateFront);
        const f32 degree = al::calcRate01(absDot, 0.0f, 0.86603f) * -90.0f * al::sign(dot);
        al::rotateVectorDegree(&front, rotateFront, normal, degree);
        al::normalize(&front);
    } else if (dot < 0.0f) {
        front = -front;
    }

    connector->attachCollision(collisionParts, front, normal, position, normal,
                               al::getGravity(actor));
}
}  // namespace

// NON_MATCHING: target 520 bytes; full 10/10 call surface recovered and bind-info radius/offset types proven by attackSensor; next hypothesis recover the original scalar vector-expression lifetime/store schedule.
void YoshiTongue::updateEatBindActor() {
    if (mEatBindInfo.isEmpty())
        return;
    if (!al::isNerve(this, &NrvYoshiTongue.Stretch) &&
        !al::isNerve(this, &NrvYoshiTongue.Hit) &&
        !al::isNerve(this, &NrvYoshiTongue.Eat))
        return;

    const s32 lastIndex = mEatBindInfo.size() - 1;
    if (lastIndex < 0)
        return;

    const sead::Vector3f tipPos = mTongueTipPos;
    const f32 rotateStep = 80.0f;
    for (s32 i = 0;; i++) {
        YoshiTongueEatBindInfo* info = mEatBindInfo.at(i);
        f32 scale = info->scale;
        if (al::isNerve(this, &NrvYoshiTongue.Eat))
            scale = al::lerpValue(info->scale, 0.25f,
                                  al::calcNerveEaseInRate(this, mParam->eatStep->value));

        al::LiveActor* actor = al::getSensorHost(info->sensor);
        const f32 radius = scale * info->radius;
        const f32 offset = scale * info->offset;

        sead::Vector3f rotatedUp = mUpDir;
        al::rotateVectorDegree(&rotatedUp, rotatedUp, mTongueDir, i * rotateStep);

        sead::Vector3f position = mTongueDir * radius;
        position += tipPos;
        position = position + rotatedUp * radius;
        position -= mUpDir * offset;
        al::resetPosition(actor, position);
        al::setScaleAll(actor, scale);

        if (i == lastIndex)
            break;
    }
}

void YoshiTongue::calcAnim() {
    al::LiveActor::calcAnim();
    sead::BoundBox3f boundingBox;
    mJointControlKeeper->calcTongueBoundingBox(&boundingBox);
    al::setDepthShadowMapBoundingBox(this, boundingBox.getMin(), boundingBox.getMax(), "Ground");
}

// NON_MATCHING: current is 1360 bytes versus the 1356-byte target with exact 37/37 semantic direct
// calls and the same 0x120 stack frame. Remaining drift is in vector-copy/snap arithmetic scheduling;
// next hypothesis is the original adjusted-direction temporary/expression order.
void YoshiTongue::startAttack(const sead::Vector3f& attackDir, const sead::Vector3f& up) {
    al::onCollide(this);
    mCollisionBuffer.clear();
    mEatBindInfo.clear();

    {
        sead::Matrix34f headMtx = sead::Matrix34f::ident;
        al::makeMtxFollowTarget(&headMtx, *al::getJointMtxPtr(mModelActor, "Head"),
                                cTongueJointTrans, cTongueJointRotate);
        al::updatePoseMtx(this, &headMtx);
    }

    sead::Vector3f* startPos = &mStartPos;
    const sead::Vector3f* trans = &al::getTrans(this);
    startPos->z = trans->z;
    sead::MemUtil::copy(startPos, trans, sizeof(sead::Vector2f));
    sead::MemUtil::copy(&mTongueTipPos, trans, sizeof(sead::Vector2f));
    mTongueTipPos.z = trans->z;
    mShrinkRestRange = 1000.0f;
    mTongueCollider->resetCollision(mTongueTipPos);
    mTipConnector->reset();
    al::setSensorRadius(this, "AttackLine", 0.0f);

    IUsePlayerHack* playerHack = *mPlayerHack;
    const f32 rotateRate =
        al::calcRate01(sead::Mathf::abs(rs::getHackStickRotateSpeed(playerHack)), 2.0f, 9.0f);
    const f32 searchAngleOffset =
        rs::isOnHackMoveStickDeepDown(*mPlayerHack) ? 15.0f : 20.0f;
    const f32 searchAngle = std::min(rotateRate * 45.0f + searchAngleOffset, 45.0f);

    mIsHack = rs::isTriggerHackSwing(playerHack);
    mIsStayClingGround = false;

    sead::Vector3f sideDir;
    sead::Vector3f sideUp;
    sead::Vector3f snapDir;
    sead::Vector3f areaCenter;
    sead::Vector3f snapPos;
    sead::Vector3f hitPos;
    sead::Vector3f hitNormal;
    sead::Vector3f tongueDir = attackDir;
    sead::Vector3f tongueUp = up;
    mEyeSensorHitHolder->findEatTargetSensor(&tongueDir, mStartPos, attackDir, up, 200.0f,
                                             searchAngle, 45.0f);

    const f32 snapDistance = mShrinkRestRange;
    al::AreaObj* snapArea = al::tryFindAreaObj(this, "YoshiTongueSnapArea", mStartPos);
    if (snapArea) {
        sideDir.set(0.0f, 0.0f, 0.0f);
        al::getAreaObjDirSide(&sideDir, snapArea);
        if (!(sead::Mathf::abs(sideDir.dot(tongueDir)) > 0.96593f)) {
            sideUp.set(0.0f, 0.0f, 0.0f);
            al::verticalizeVec(&sideUp, sideDir, tongueUp);
            if (!al::tryNormalizeOrZero(&sideUp)) {
                sideUp = tongueUp * al::sign(sideDir.dot(tongueUp));
                al::normalize(&sideUp);
            }

            snapDir.set(0.0f, 0.0f, 0.0f);
            al::verticalizeVec(&snapDir, sideDir, tongueDir);
            if (!al::tryNormalizeOrZero(&snapDir)) {
                snapDir = sideUp.cross(sideDir);
                snapDir *= al::sign(sideDir.dot(tongueDir));
                al::normalize(&snapDir);
            }

            areaCenter.set(0.0f, 0.0f, 0.0f);
            al::calcAreaObjCenterPos(&areaCenter, snapArea);
            snapPos = mStartPos + snapDir * snapDistance;
            hitPos.set(0.0f, 0.0f, 0.0f);
            hitNormal.set(0.0f, 0.0f, 0.0f);
            const sead::Vector3f& adjustedPos =
                al::checkAreaObjCollisionByArrow(&hitPos, &hitNormal, snapArea, mStartPos, snapPos)
                    ? hitPos
                    : snapPos;
            sead::Vector3f adjustedDir =
                adjustedPos + sideDir * (areaCenter - adjustedPos).dot(sideDir) - mStartPos;
            if (al::tryNormalizeOrZero(&adjustedDir)) {
                snapDir.z = adjustedDir.z;
                sead::MemUtil::copy(&snapDir, &adjustedDir, sizeof(sead::Vector2f));
            }

            sead::Quatf rotation = sead::Quatf::unit;
            al::makeQuatFrontUp(&rotation, snapDir, sideUp);
            al::calcQuatFront(&tongueDir, rotation);
            al::calcQuatUp(&tongueUp, rotation);
        }
    }

    mTongueDir = tongueDir;
    al::verticalizeVec(&mUpDir, mTongueDir, tongueUp);
    al::normalize(&mUpDir);
    rs::resetJudge(mJudgeStartClingFix);
    mJointControlKeeper->update(mTongueDir, mUpDir, mTongueTipPos);
    al::setNerve(this, &NrvYoshiTongue.Stretch);
    makeActorAlive();
    al::startHitReaction(this, "舌を伸ばす");
    al::startHitReaction(mHost, "舌を伸ばす");
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
    return al::isGreaterStep(this, mParam->eatStep->value);
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
    sead::MemUtil::copy(tipPos, &mTongueTipPos, sizeof(sead::Vector2f));
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
    sead::MemUtil::copy(direction, &pullDirection, sizeof(sead::Vector2f));
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
    sead::MemUtil::copy(tipPos, direction, sizeof(sead::Vector2f));
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

// NON_MATCHING: target-sized 608/608 with exact 14/14 semantic direct calls; remaining mismatch
// is the first-step start/tip vector-copy and deceleration FP register schedule. Next hypothesis:
// original vector-copy/calculation declaration order.
void YoshiTongue::exeStretch() {
    const al::LiveActor* modelActor = mModelActor;
    sead::Matrix34f headMtx = sead::Matrix34f::ident;
    al::makeMtxFollowTarget(&headMtx, *al::getJointMtxPtr(modelActor, "Head"),
                            cTongueJointTrans, cTongueJointRotate);
    al::updatePoseMtx(this, &headMtx);

    if (al::isFirstStep(this)) {
        sead::Vector3f* startPos = &mStartPos;
        const sead::Vector3f* trans = &al::getTrans(this);
        startPos->z = trans->z;
        sead::MemUtil::copy(startPos, trans, sizeof(sead::Vector2f));
        mTongueTipPos.z = trans->z;
        sead::MemUtil::copy(&mTongueTipPos, trans, sizeof(sead::Vector2f));
        al::resetPosition(this, *startPos);

        const f32 speed = getTongueParamSpeed();
        const f32 stretchStep = mParam->stretchStep->value;
        const f32 deceleration = speed / stretchStep;
        _1e0 = (s32)((getTongueParamRange() -
                      (speed * stretchStep + stretchStep * (stretchStep * (deceleration * -0.5f)))) /
                     speed);
        _1e4 = -deceleration;
        mVelocity = mTongueDir * getTongueParamSpeed();
    }

    if (rs::isTriggerHackAction(*mPlayerHack))
        mIsStayClingGround = true;

    if (al::isGreaterEqualStep(this, _1e0))
        mVelocity += mTongueDir * _1e4;

    if (reactionCollideWall() || reactionCollideGround())
        return;

    if (tryResetTongueCollision(&mTongueTipPos, mTongueCollider, this)) {
        mVelocity.set(0.0f, 0.0f, 0.0f);
        if (isExistEatBind())
            al::setNerve(this, &NrvYoshiTongue.Eat);
        else
            al::setNerve(this, &NrvYoshiTongue.Return);
        return;
    }

    if (al::isNearZeroOrLess(mTongueDir.dot(mVelocity), 0.001f))
        al::setNerve(this, &NrvYoshiTongue.Hit);
}

f32 YoshiTongue::getTongueParamSpeed() const {
    const bool isHack = mIsHack;
    const f32 speed = mParam->speed->value;
    if (!isHack)
        return speed;
    return speed + speed;
}

f32 YoshiTongue::getTongueParamRange() const {
    return mParam->range->value;
}

// NON_MATCHING: 496 bytes vs target 508 with the same 17 semantic calls, but current Clang
// hoists the single shared setNerve call into the cancel fast path while target places it at the
// physical tail. Next hypothesis: original next-nerve branch/declaration shape that inhibits hoist.
bool YoshiTongue::reactionCollideWall() {
    if (!rs::isCollidedWall(mTongueCollider))
        return false;

    mVelocity.set(0.0f, 0.0f, 0.0f);
    const al::Nerve* nextNerve = nullptr;
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
            nextNerve = &NrvYoshiTongue.Eat;
        else
            nextNerve = &NrvYoshiTongue.Return;
    } else {
        mTipConnector->tryCalcConnect(&mTongueDir, &mUpDir, &mTongueTipPos);
        if ((rs::isHoldHackAction(*mPlayerHack) || mIsHack) && !isExistEatBind()) {
            al::startHitReaction(this, "壁接触によるくっつき");
            mJudgeStartClingFix->setCheckWall();
            rs::updateJudgeAndResult(mJudgeStartClingFix);
            nextNerve = &NrvYoshiTongue.ClingWall;
        } else if (al::isNerve(this, &NrvYoshiTongue.Hit)) {
            al::startHitReaction(this, "壁接触による伸びキャンセル");
            if (isExistEatBind())
                nextNerve = &NrvYoshiTongue.Eat;
            else
                nextNerve = &NrvYoshiTongue.Return;
        } else {
            al::startHitReaction(this, "壁接触");
            nextNerve = &NrvYoshiTongue.Hit;
        }
    }

    al::setNerve(this, nextNerve);
    return true;
}

bool YoshiTongue::reactionCollideGround() {
    if (!rs::isCollidedGround(mTongueCollider))
        return false;

    if (!mIsStayClingGround) {
        if (!rs::isHoldHackAction(*mPlayerHack) && !mIsHack)
            return false;

        if (!rs::isCollidedGroundOverAngle(this, mTongueCollider,
                                            mPlayerConst->getStandAngleMin()))
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

namespace {
bool tryResetTongueCollision(sead::Vector3f* position, YoshiTongueCollider* collider,
                             const al::LiveActor* actor) {
    const sead::Vector3f& trans = al::getTrans(actor);
    f32 distanceLength = 0.0f;
    sead::Vector3f direction(0.0f, 0.0f, 0.0f);
    al::separateScalarAndDirection(&distanceLength, &direction, *position - trans);
    distanceLength = sead::Mathf::max(0.0f, distanceLength - 5.0f);
    if (al::isNearZero(distanceLength))
        return false;

    if (!alCollisionUtil::getHitPosOnArrow(actor, position, trans, direction * distanceLength,
                                            nullptr, nullptr))
        return false;

    collider->resetCollision(*position);
    return true;
}
}  // namespace

void YoshiTongue::returnOrEatHide() {
    if (isExistEatBind())
        al::setNerve(this, &NrvYoshiTongue.Eat);
    else
        al::setNerve(this, &NrvYoshiTongue.Return);
}

// NON_MATCHING: 1044 bytes vs target 1048 with exact 23/23 semantic direct calls; direct zero
// construction now reproduces the target cling-position zero stores. The remaining one-instruction
// delta is in the first-step return-offset assignment. Next hypothesis: canonical SEAD vector helper
// form that preserves the target component-load schedule.
void YoshiTongue::exeStay() {
    if (al::isFirstStep(this)) {
        mReturnOffset = mTongueTipPos - al::getTrans(this);
        _214 = 0;
    }

    {
        const al::LiveActor* modelActor = mModelActor;
        sead::Matrix34f headMtx = sead::Matrix34f::ident;
        al::makeMtxFollowTarget(&headMtx, *al::getJointMtxPtr(modelActor, "Head"),
                                cTongueJointTrans, cTongueJointRotate);
        al::updatePoseMtx(this, &headMtx);
    }

    if (reactionCollideWall() || reactionCollideGround())
        return;

    if (tryResetTongueCollision(&mTongueTipPos, mTongueCollider, this)) {
        mVelocity.set(0.0f, 0.0f, 0.0f);
        if (isExistEatBind())
            al::setNerve(this, &NrvYoshiTongue.Eat);
        else
            al::setNerve(this, &NrvYoshiTongue.Return);
        return;
    }

    if (mIsStayClingGround) {
        if (_214 > 0 || rs::isHoldHackAction(*mPlayerHack)) {
            if (!isExistEatBind()) {
                sead::Vector3f clingPos(0.0f, 0.0f, 0.0f);
                if (rs::findClingGroundPos(&clingPos, mHost, mTongueTipPos, 180.0f)) {
                    const sead::Vector3f& trans = al::getTrans(this);
                    sead::Vector3f currentDir = mTongueTipPos - trans;
                    if (al::tryNormalizeOrZero(&currentDir)) {
                        sead::Vector3f clingDir = clingPos - trans;
                        if (al::tryNormalizeOrZero(&clingDir) &&
                            !al::isParallelDirection(currentDir, clingDir)) {
                            sead::Quatf rotation = sead::Quatf::unit;
                            al::makeQuatRotationLimit(&rotation, currentDir, clingDir,
                                                      sead::Mathf::deg2rad(5.0f));
                            mReturnOffset.setRotated(rotation, mReturnOffset);
                        }
                    }
                }
            }
            --_214;
        }
    } else {
        if (rs::isTriggerHackAction(*mPlayerHack))
            mIsStayClingGround = true;
        if (rs::isTriggerHackSwing(*mPlayerHack)) {
            _214 = 30;
            mIsStayClingGround = true;
        }
    }

    mVelocity = al::getTrans(this) + mReturnOffset - mTongueTipPos;
    rs::cutVectorCollision(&mVelocity, mTongueCollider, 1.0f);
    mReturnOffset = mTongueTipPos + mVelocity - al::getTrans(this);

    if (!al::isLessStep(this, mParam->clingWallStep->value)) {
        if (isExistEatBind())
            al::setNerve(this, &NrvYoshiTongue.Eat);
        else
            al::setNerve(this, &NrvYoshiTongue.Return);
        return;
    }

    if (isExistEatBind())
        al::setNerve(this, &NrvYoshiTongue.Eat);
}

void YoshiTongue::exeHit() {}

// NON_MATCHING: target 1564 bytes vs current 1480; 37/37 semantic calls, 0x130 frame, unordered comparisons, and FMAX clamps recovered. Next hypothesis: recover the target FP callee-save placement for wallAlong/tip/history values across checkHitLinePlane.
void YoshiTongue::exeClingWall() {
    const al::LiveActor* modelActor = mModelActor;
    {
        sead::Matrix34f headMtx = sead::Matrix34f::ident;
        al::makeMtxFollowTarget(&headMtx, *al::getJointMtxPtr(modelActor, "Head"),
                                cTongueJointTrans, cTongueJointRotate);
        al::updatePoseMtx(this, &headMtx);
    }

    if (!mTipConnector->tryCalcConnect(&mTongueDir, &mUpDir, &mTongueTipPos)) {
        al::setNerve(this, &NrvYoshiTongue.Return);
        return;
    }

    YoshiTongueTipConnector* connector = mTipConnector;
    if (tryResetTongueCollision(&mTongueTipPos, mTongueCollider, this)) {
        al::setNerve(this, &NrvYoshiTongue.Return);
        return;
    }

    sead::Vector3f direction = al::getTrans(this) - mTongueTipPos;
    if (al::tryNormalizeOrZero(&direction) && !connector->isGroundAttached() &&
        !(direction.dot(mUpDir) > 0.087156f)) {
        al::setNerve(this, &NrvYoshiTongue.Return);
        return;
    }

    if (al::isFirstStep(this)) {
        const sead::Vector3f& trans = al::getTrans(this);
        mShrinkRestRange = sead::Mathf::max(150.0f, (mTongueTipPos - trans).length());
        mReturnOffset = mTongueTipPos - al::getTrans(this);
    }

    if (!rs::updateJudgeAndResult(mJudgeStartClingFix)) {
        if (!rs::isCollidedWall(mTongueCollider)) {
            al::setNerve(this, &NrvYoshiTongue.Return);
            return;
        }

        if (mWallActionHistory->isJumpStored()) {
            const sead::Vector3f& wallNormal = rs::getCollidedWallNormal(mTongueCollider);
            sead::Vector3f wallSide = wallNormal.cross(al::getGravity(this));
            al::normalize(&wallSide);
            sead::Vector3f wallAlong = wallNormal.cross(wallSide);
            al::normalize(&wallAlong);

            const sead::Vector3f historyPos = mWallActionHistory->getJumpWallPosition();
            const sead::Vector3f tipPos = mTongueTipPos;
            sead::Vector3f lineDir = mReturnOffset;
            if (!al::tryNormalizeOrZero(&lineDir))
                lineDir = -wallNormal;

            sead::Vector3f lineHit(0.0f, 0.0f, 0.0f);
            if (al::checkHitLinePlane(&lineHit, al::getTrans(this), lineDir, mTongueTipPos,
                                      wallNormal)) {
                f32 moveDistance = (lineHit - tipPos).dot(wallAlong);
                const f32 maxMove =
                    sead::Mathf::max(0.0f, (historyPos - tipPos).dot(wallAlong)) + 5.0f;
                moveDistance = sead::Mathf::clamp(moveDistance, 5.0f, maxMove);
                const sead::Vector3f movePos = tipPos + wallAlong * moveDistance;

                sead::Vector3f rayDir = -wallNormal;
                al::verticalizeVec(&rayDir, al::getGravity(this), rayDir);
                al::normalize(&rayDir);
                const sead::Vector3f& gravity = al::getGravity(this);
                al::TriangleFilterWallOnly filter(gravity);
                const sead::Vector3f rayStart = movePos - rayDir * 50.0f;
                const sead::Vector3f rayDelta = rayDir * 100.0f;
                const al::ArrowHitInfo* hitInfo = nullptr;
                if (alCollisionUtil::getFirstPolyOnArrow(this, &hitInfo, rayStart, rayDelta,
                                                         nullptr, &filter)) {
                    mTongueTipPos = alCollisionUtil::getCollisionHitPos(**hitInfo);
                    attachTongueTipCollision(connector, this,
                                             alCollisionUtil::getCollisionHitParts(**hitInfo),
                                             alCollisionUtil::getCollisionHitNormal(**hitInfo),
                                             mTongueTipPos, mTongueDir);
                } else {
                    mTongueTipPos = movePos;
                }

                mTongueCollider->resetCollision(mTongueTipPos);
                f32 currentRange = (mTongueTipPos - al::getTrans(this)).length();
                if (!(currentRange < mShrinkRestRange))
                    currentRange = mShrinkRestRange;
                mShrinkRestRange = sead::Mathf::max(150.0f, currentRange);
                mReturnOffset = mTongueTipPos - al::getTrans(this);
            }
        }
    }

    if (al::isGreaterEqualStep(this, 1) && rs::isOnGround(mHost, mCollision))
        al::setNerve(this, &NrvYoshiTongue.Return);
}

void YoshiTongue::exeClingGround() {
    const al::LiveActor* modelActor = mModelActor;
    {
        sead::Matrix34f headMtx = sead::Matrix34f::ident;
        al::makeMtxFollowTarget(&headMtx, *al::getJointMtxPtr(modelActor, "Head"),
                                cTongueJointTrans, cTongueJointRotate);
        al::updatePoseMtx(this, &headMtx);
    }

    if (!mTipConnector->tryCalcConnect(&mTongueDir, &mUpDir, &mTongueTipPos)) {
        al::setNerve(this, &NrvYoshiTongue.Return);
        return;
    }

    YoshiTongueTipConnector* connector = mTipConnector;
    if (tryResetTongueCollision(&mTongueTipPos, mTongueCollider, this)) {
        al::setNerve(this, &NrvYoshiTongue.Return);
        return;
    }

    sead::Vector3f direction = al::getTrans(this) - mTongueTipPos;
    if (al::tryNormalizeOrZero(&direction) && !connector->isGroundAttached() &&
        !(direction.dot(mUpDir) > 0.087156f)) {
        al::setNerve(this, &NrvYoshiTongue.Return);
        return;
    }

    if (al::isFirstStep(this)) {
        mShrinkRestRange = sead::Mathf::max(150.0f, (mTongueTipPos - al::getTrans(this)).length());
        mReturnOffset = mTongueTipPos - al::getTrans(this);
    }

    if ((!rs::updateJudgeAndResult(mJudgeStartClingFix) &&
         !rs::isCollidedGround(mTongueCollider)) ||
        al::isGreaterEqualStep(this, 1))
        al::setNerve(this, &NrvYoshiTongue.Return);
}

void YoshiTongue::exeShrink() {
    const al::LiveActor* modelActor = mModelActor;
    {
        sead::Matrix34f headMtx = sead::Matrix34f::ident;
        al::makeMtxFollowTarget(&headMtx, *al::getJointMtxPtr(modelActor, "Head"),
                                cTongueJointTrans, cTongueJointRotate);
        al::updatePoseMtx(this, &headMtx);
    }

    if (!mTipConnector->tryCalcConnect(&mTongueDir, &mUpDir, &mTongueTipPos)) {
        al::setNerve(this, &NrvYoshiTongue.Return);
        return;
    }

    YoshiTongueTipConnector* connector = mTipConnector;
    if (tryResetTongueCollision(&mTongueTipPos, mTongueCollider, this)) {
        al::setNerve(this, &NrvYoshiTongue.Return);
        return;
    }

    sead::Vector3f direction = al::getTrans(this) - mTongueTipPos;
    if (!al::tryNormalizeOrZero(&direction) || connector->isGroundAttached())
        return;
    if (direction.dot(mUpDir) > 0.087156f)
        return;
    al::setNerve(this, &NrvYoshiTongue.Return);
}

// NON_MATCHING: target-sized 396/396; current uses the same 0x60 frame but assigns offset to the upper stack slot while target places offset at sp and reuses the matrix slot for direction. Next hypothesis: original local declaration/lifetime order that yields target stack coloring.
void YoshiTongue::exeReturn() {
    if (al::isFirstStep(this)) {
        mReturnOffset = mTongueTipPos - al::getTrans(this);
        al::offCollide(this);
    }

    sead::Vector3f offset;
    offset.set(0.0f, 0.0f, 0.0f);
    const al::LiveActor* modelActor = mModelActor;
    {
        sead::Matrix34f headMtx = sead::Matrix34f::ident;
        al::makeMtxFollowTarget(&headMtx, *al::getJointMtxPtr(modelActor, "Head"),
                                cTongueJointTrans, cTongueJointRotate);
        al::updatePoseMtx(this, &headMtx);
    }

    al::lerpVec(&offset, mReturnOffset, sead::Vector3f::zero,
                al::calcNerveEaseOutRate(this, mParam->eatStep->value));
    mVelocity = al::getTrans(this) + offset - mTongueTipPos;

    sead::Vector3f direction = offset;
    if (al::tryNormalizeOrZero(&direction))
        calcTongueDirection(&mTongueDir, &mUpDir, this, direction);

    if (!al::isLessStep(this, mParam->eatStep->value))
        al::setNerve(this, &NrvYoshiTongue.Stay);
}

namespace {
// NON_MATCHING: 296 bytes vs target 308; direct actor-up zeroing is corpus-faithful, but compact baseDir selection elides target side/front temporary stores. Next hypothesis: original basis-selection source form that preserves branch-local front storage without duplicating the cross product.
void calcTongueDirection(sead::Vector3f* tongueDir, sead::Vector3f* upDir,
                         al::LiveActor* actor, const sead::Vector3f& direction) {
    sead::Vector3f actorUp;
    actorUp.set(0.0f, 0.0f, 0.0f);
    al::calcUpDir(&actorUp, actor);

    sead::Vector3f baseDir = actorUp;
    if (al::isParallelDirection(actorUp, direction))
        al::calcFrontDir(&baseDir, actor);

    sead::Vector3f sideDir = direction.cross(baseDir);
    al::normalize(&sideDir);
    *tongueDir = direction;
    al::normalize(tongueDir);
    *upDir = sideDir.cross(direction);
    al::normalize(upDir);
}
}  // namespace

// NON_MATCHING: target-sized 356/356; direct offset zeroing and model-actor lifetime recover target size, but stack coloring still swaps the offset and matrix/direction slots. Next hypothesis: original local declaration/lifetime order that places offset at sp.
void YoshiTongue::exeEat() {
    if (al::isFirstStep(this)) {
        mReturnOffset = mTongueTipPos - al::getTrans(this);
        al::offCollide(this);
    }

    sead::Vector3f offset;
    offset.set(0.0f, 0.0f, 0.0f);
    const al::LiveActor* modelActor = mModelActor;
    {
        sead::Matrix34f headMtx = sead::Matrix34f::ident;
        al::makeMtxFollowTarget(&headMtx, *al::getJointMtxPtr(modelActor, "Head"),
                                cTongueJointTrans, cTongueJointRotate);
        al::updatePoseMtx(this, &headMtx);
    }

    al::lerpVec(&offset, mReturnOffset, sead::Vector3f::zero,
                al::calcNerveEaseOutRate(this, mParam->eatStep->value));
    mVelocity = al::getTrans(this) + offset - mTongueTipPos;

    sead::Vector3f direction = offset;
    if (al::tryNormalizeOrZero(&direction))
        calcTongueDirection(&mTongueDir, &mUpDir, this, direction);
}

void YoshiTongue::exeHide() {
    makeActorDead();
}

void YoshiTongue::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isNerve(this, &NrvYoshiTongue.Stretch) && al::isLessEqualStep(this, 0) &&
        al::isSensorName(self, "AttackBottom"))
        return;

    if (al::isSensorName(self, "AttackLine")) {
        sead::Vector3f sensorOffset = al::getSensorPos(other) - al::getTrans(this);
        al::verticalizeVec(&sensorOffset, mFaceDir, sensorOffset);
        if (sensorOffset.length() - al::getSensorRadius(other) > 50.0f)
            return;
    }

    if (al::isSensorPlayerAttack(self) &&
        (al::isNerve(this, &NrvYoshiTongue.Stretch) || al::isNerve(this, &NrvYoshiTongue.Hit) ||
         al::isNerve(this, &NrvYoshiTongue.Eat))) {
        f32 radius = al::getSensorRadius(other);
        f32 offset = al::getSensorRadius(other);
        f32 scale = 1.0f;
        if (rs::sendMsgYoshiTongueEatBind(other, self, &radius, &offset, &scale)) {
            const s32 bindIndex = mEatBindInfo.size();
            mEatBindInfoBuffer[bindIndex]->sensor = other;
            mEatBindInfoBuffer[bindIndex]->scale = scale;
            mEatBindInfoBuffer[bindIndex]->radius = radius;
            mEatBindInfoBuffer[bindIndex]->offset = offset;
            mEatBindInfo.pushBack(mEatBindInfoBuffer[bindIndex]);
            return;
        }
    }

    if (al::isSensorPlayerAttack(self) && !al::isNerve(this, &NrvYoshiTongue.Hide) &&
        !al::isNerve(this, &NrvYoshiTongue.Stay)) {
        bool isAlreadyHit = false;
        if (mCollisionBuffer) {
            al::LiveActor* sensorHost = al::getSensorHost(other);
            for (s32 i = 0; i < mCollisionBuffer.size(); i++) {
                if (mCollisionBuffer(i) == sensorHost) {
                    isAlreadyHit = true;
                    break;
                }
            }
        }

        if (!isAlreadyHit && rs::sendMsgYoshiTongueAttack(other, self)) {
            al::LiveActor* sensorHost = al::getSensorHost(other);
            mCollisionBuffer.forcePushBack(sensorHost);

            if (!al::isNerve(this, &NrvYoshiTongue.Return) &&
                !al::isNerve(this, &NrvYoshiTongue.Eat)) {
                if (mEatBindInfo.isEmpty())
                    al::setNerve(this, &NrvYoshiTongue.Return);
                else
                    al::setNerve(this, &NrvYoshiTongue.Eat);
                return;
            }
        }
    }

    if (al::isSensorPlayerAttack(self) && !al::isNerve(this, &NrvYoshiTongue.Hide) &&
        !al::isNerve(this, &NrvYoshiTongue.Stay))
        rs::sendMsgWeaponItemGet(other, self);

}

bool YoshiTongue::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                             al::HitSensor* self) {
    return false;
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
