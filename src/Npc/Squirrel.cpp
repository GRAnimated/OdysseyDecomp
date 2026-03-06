#include "Npc/Squirrel.h"

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Base/StringUtil.h"
#include "Library/Collision/CollisionPartsKeeperUtil.h"
#include "Library/Collision/CollisionPartsTriangle.h"
#include "Library/Collision/PartsConnector.h"
#include "Library/Collision/PartsConnectorUtil.h"
#include "Library/Joint/JointSpringControllerHolder.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorResourceFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Shadow/ActorShadowUtil.h"
#include "Library/Yaml/ByamlUtil.h"

#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "Util/ItemGenerator.h"
#include "Util/PlayerUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(Squirrel, Wait);
NERVE_IMPL(Squirrel, Hide);
NERVE_IMPL(Squirrel, Appear);
NERVE_IMPL(Squirrel, Turn);
NERVE_IMPL(Squirrel, Disappear);
NERVE_IMPL(Squirrel, Runaway);
NERVE_IMPL(Squirrel, RestartPrepare);

NERVES_MAKE_STRUCT(Squirrel, Wait, Hide, Appear, Turn, Disappear);
NERVES_MAKE_NOSTRUCT(Squirrel, Runaway, RestartPrepare);

bool isNearPlayerOrCap(const al::LiveActor* actor, f32 findDistance) {
    if (rs::isNearPlayerH(actor, findDistance))
        return true;

    sead::Vector3f capPos = {0.0f, 0.0f, 0.0f};
    if (!rs::tryGetFlyingCapPos(&capPos, actor))
        return false;

    sead::Vector3f trans = al::getTrans(actor);
    sead::Vector3f diff = capPos - trans;
    al::verticalizeVec(&diff, al::getGravity(actor), diff);
    return diff.length() < findDistance;
}

void calcRunawayDir(sead::Vector3f* outDir, bool* isLeftRight, const al::LiveActor* actor) {
    sead::Vector3f downDir = {0.0f, 0.0f, 0.0f};
    al::calcDownDir(&downDir, actor);

    sead::Vector3f trans = al::getTrans(actor);
    sead::Vector3f playerPos = rs::getPlayerPos(actor);
    sead::Vector3f diff = trans - playerPos;

    al::verticalizeVec(outDir, downDir, diff);
    if (!al::tryNormalizeOrZero(outDir))
        al::calcBackDir(outDir, actor);

    sead::Vector3f upDir = {-downDir.x, -downDir.y, -downDir.z};
    f32 angle = al::getRandom(15.0f, 45.0f);
    f32 sign = 1.0f;
    if (*isLeftRight)
        sign = -1.0f;
    al::rotateVectorDegree(outDir, *outDir, upDir, angle * sign);
    *isLeftRight ^= true;
}

void updateRunawayMove(al::LiveActor* actor, const f32* params) {
    sead::Vector3f frontDir = {0.0f, 0.0f, 0.0f};
    sead::Vector3f downDir = {0.0f, 0.0f, 0.0f};
    al::calcFrontDir(&frontDir, actor);
    al::calcDownDir(&downDir, actor);

    if (al::isCollidedGround(actor)) {
        const sead::Vector3f& groundNormal = al::getCollidedGroundNormal(actor);
        sead::Vector3f negNormal = {-groundNormal.x, -groundNormal.y, -groundNormal.z};
        al::turnVecToVecRate(&downDir, downDir, negNormal, 0.1f);
        al::normalize(&downDir);
        al::verticalizeVec(&frontDir, downDir, frontDir);
        al::normalize(&frontDir);

        sead::Vector3f upDir = {-downDir.x, -downDir.y, -downDir.z};
        sead::Quatf quat = sead::Quatf::unit;
        al::makeQuatFrontUp(&quat, frontDir, upDir);
        al::updatePoseQuat(actor, quat);

        if (al::isExistShadowMaskCtrl(actor)) {
            sead::Vector3f shadowDir = {0.0f, 0.0f, 0.0f};
            al::calcDownDir(&shadowDir, actor);
            al::setShadowMaskDropDir(actor, shadowDir);
        }
    }

    f32 accel = params[2];
    f32 velX = accel * frontDir.x + downDir.x * 10.0f;
    f32 velY = accel * frontDir.y + downDir.y * 10.0f;
    f32 velZ = accel * frontDir.z + downDir.z * 10.0f;

    f32 waterScale = 1.0f;
    if (al::isInWater(actor))
        waterScale = 0.5f;

    sead::Vector3f velocity = {velX * waterScale, velY * waterScale, velZ * waterScale};
    al::addVelocity(actor, velocity);
    al::scaleVelocity(actor, params[3]);
    al::limitVelocityDirSign(actor, downDir, 10.0f);
}

}  // namespace

// NON_MATCHING: field init order and coalescing differ; regswap
Squirrel::Squirrel(const char* name) : al::LiveActor(name) {
    mIsLeftRight = al::isHalfProbability();
}

// NON_MATCHING: Vector3f copy pattern (64+32 vs 3×32 bit); 95% match
void Squirrel::init(const al::ActorInitInfo& info) {
    al::initActor(this, info);
    al::initNerve(this, &NrvSquirrel.Wait, 0);
    al::offCollide(this);

    mInitTrans = al::getTrans(this);
    al::calcQuat(&mInitQuat, this);

    bool turnL = al::isExistAction(this, "TurnL");
    mIsExistTurnLR = turnL & al::isExistAction(this, "TurnR");
    bool waitA = al::isExistAction(this, "WaitA");
    mIsExistWaitAB = waitA & al::isExistAction(this, "WaitB");
    mIsExistWaitRandom = al::isExistAction(this, "WaitRandom");
    bool waitRandomA = al::isExistAction(this, "WaitRandomA");
    mIsExistWaitRandomAB = waitRandomA & al::isExistAction(this, "WaitRandomB");

    al::ByamlIter iter;
    if (al::tryGetActorInitFileIter(&iter, this, "InitRunawayAnimalParam", nullptr)) {
        al::tryGetByamlF32(&mFindDistance, iter, "FindDistance");
        al::tryGetByamlF32(&mTurnSpeed, iter, "TurnSpeed");
        al::tryGetByamlF32(&mRunAccel, iter, "RunAccel");
        al::tryGetByamlF32(&mFriction, iter, "Friction");
        al::tryGetByamlS32(&mMoveStep, iter, "MoveStep");
        if (mIsExistWaitRandom) {
            al::tryGetByamlF32(&mRandomWaitProbA, iter, "RandomWaitProbabilityA");
        } else if (mIsExistWaitRandomAB) {
            al::tryGetByamlF32(&mRandomWaitProbA, iter, "RandomWaitProbabilityA");
            al::tryGetByamlF32(&mRandomWaitProbB, iter, "RandomWaitProbabilityB");
        }
        al::tryGetByamlBool(&mIsValidReflectWall, iter, "IsValidReflectWall");
        al::tryGetByamlBool(&mIsTurnOnPoint, iter, "IsTurnOnPoint");
    }

    mConnector = al::createCollisionPartsConnector(this, al::getQuat(this));
    mItemGenerator = new ItemGenerator(this, info);

    if (al::tryGetArg(&mIsHide, info, "IsHide") && mIsHide) {
        al::hideModelIfShow(this);
        al::setNerve(this, &NrvSquirrel.Hide);
    }

    al::tryGetArg(&mRunawayCount, info, "RunawayCount");
    al::createAndSetColliderSpecialPurpose(this, "ShibakenMoveLimit");
    al::JointSpringControllerHolder::tryCreateAndInitJointControllerKeeper(
        this, "InitJointSpringCtrl");

    if (al::isEqualString(al::getModelName(this), "RunawayCrab")) {
        GameDataHolderAccessor accessor(this);
        const char* colorName;
        if (GameDataFunction::isWorldSea(accessor))
            colorName = "Gold";
        else
            colorName = "Pink";
        al::startMclAnimAndSetFrameAndStop(this, colorName, 0.0f);
        al::startMtpAnimAndSetFrameAndStop(this, colorName, 0.0f);
    }

    _12c = al::isNerve(this, &NrvSquirrel.Wait);
    al::trySyncStageSwitchAppear(this);
}

// NON_MATCHING: instruction reordering in f32 computation; regswap
void Squirrel::initAfterPlacement() {
    sead::Vector3f hitPos = {0.0f, 0.0f, 0.0f};
    sead::Vector3f upDir = {0.0f, 0.0f, 0.0f};
    al::calcUpDir(&upDir, this);

    al::Triangle triangle;
    const sead::Vector3f& trans = al::getTrans(this);
    sead::Vector3f startPos = {trans.x + upDir.x * 50.0f, trans.y + upDir.y * 50.0f,
                               trans.z + upDir.z * 50.0f};
    sead::Vector3f dir = {upDir.x * -150.0f, upDir.y * -150.0f, upDir.z * -150.0f};

    alCollisionUtil::getFirstPolyOnArrow(this, &hitPos, &triangle, startPos, dir, nullptr,
                                         nullptr);
    al::resetPosition(this, hitPos);
    al::attachMtxConnectorToCollisionParts(mConnector, triangle.getCollisionParts());
    al::setConnectorBaseQuatTrans(al::getQuat(this), al::getTrans(this), mConnector);
}

void Squirrel::control() {
    mItemGenerator->tryUpdateHintTransIfExistShine();
    al::updateMaterialCodeWater(this);
}

void Squirrel::appear() {
    al::LiveActor::appear();
    if (al::isNerve(this, &NrvSquirrel.Hide))
        al::setNerve(this, &NrvSquirrel.Appear);
}

void Squirrel::kill() {
    al::startHitReaction(this, u8"消滅");
    al::LiveActor::kill();
}

// NON_MATCHING: stack layout differs; regswap; instruction reordering
bool Squirrel::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) {
    if (rs::isMsgPlayerDisregardHomingAttack(msg))
        return true;

    if (al::isNerve(this, &NrvSquirrel.Wait) && rs::isMsgBlowDown(msg)) {
        sead::Vector3f diff;
        sead::Vector3f downDir = {0.0f, 0.0f, 0.0f};
        al::calcDownDir(&downDir, this);

        const sead::Vector3f& selfTrans = al::getActorTrans(self);
        const sead::Vector3f& otherTrans = al::getActorTrans(other);
        diff = selfTrans - otherTrans;

        al::verticalizeVec(&mRunawayDir, downDir, diff);
        if (!al::tryNormalizeOrZero(&mRunawayDir))
            al::calcBackDir(&mRunawayDir, this);

        sead::Vector3f upDir = {-downDir.x, -downDir.y, -downDir.z};
        f32 angle = al::getRandom(15.0f, 45.0f);
        f32 sign = 1.0f;
        if (mIsLeftRight)
            sign = -1.0f;
        al::rotateVectorDegree(&mRunawayDir, mRunawayDir, upDir, angle * sign);
        mIsLeftRight ^= true;
        al::setNerve(this, &NrvSquirrel.Turn);
    }

    return false;
}

void Squirrel::exeHide() {
    if (al::isFirstStep(this))
        al::setVelocityZero(this);

    if (isNearPlayerOrCap(this, mFindDistance)) {
        al::invalidateClipping(this);
        al::setNerve(this, &NrvSquirrel.Appear);
    }
}

void Squirrel::exeAppear() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Appear");
        al::setVelocityZero(this);
        al::showModelIfHide(this);
    }

    al::MtxConnector* connector = mConnector;
    al::calcConnectQT(al::getQuatPtr(this), al::getTransPtr(this), connector);

    if (al::isActionEnd(this)) {
        al::validateClipping(this);
        if (isNearPlayerOrCap(this, mFindDistance)) {
            calcRunawayDir(&mRunawayDir, &mIsLeftRight, this);
            al::setNerve(this, &NrvSquirrel.Turn);
        } else {
            _12c = false;
            al::setNerve(this, &NrvSquirrel.Wait);
        }
    }
}

// NON_MATCHING: one fadd vs mov mismatch in modf argument setup
void Squirrel::exeWait() {
    if (al::isFirstStep(this)) {
        if (mIsExistWaitAB) {
            al::startAction(this, al::isHalfProbability() ? "WaitA" : "WaitB");
        } else if (_12c) {
            al::startActionAtRandomFrame(this, "Wait");
            _12c = false;
        } else {
            al::startAction(this, "Wait");
        }
        al::setVelocityZero(this);
        if (al::isExistShadowMaskCtrl(this)) {
            sead::Vector3f shadowDir = {0.0f, 0.0f, 0.0f};
            al::calcDownDir(&shadowDir, this);
            al::setShadowMaskDropDir(this, shadowDir);
        }
    }

    al::MtxConnector* connector = mConnector;
    al::calcConnectQT(al::getQuatPtr(this), al::getTransPtr(this), connector);

    if (mIsExistWaitRandom || mIsExistWaitRandomAB) {
        if (al::isActionOneTime(this)) {
            if (al::isActionEnd(this)) {
                if (mIsExistWaitAB) {
                    al::startAction(this, al::isHalfProbability() ? "WaitA" : "WaitB");
                } else if (_12c) {
                    al::startActionAtRandomFrame(this, "Wait");
                    _12c = false;
                } else {
                    al::startAction(this, "Wait");
                }
            }
        } else {
            f32 frame = al::getActionFrame(this);
            f32 nextFrame = frame + al::getActionFrameRate(this);
            f32 maxFrame = al::getActionFrameMax(this, al::getActionName(this));
            f32 wrappedFrame = al::modf(nextFrame, maxFrame) + 0.0f;
            if (wrappedFrame < al::getActionFrame(this)) {
                f32 rnd = al::getRandom();
                if (mIsExistWaitRandom) {
                    if (rnd < mRandomWaitProbA)
                        al::startAction(this, "WaitRandom");
                } else if (rnd < mRandomWaitProbA) {
                    al::startAction(this, "WaitRandomA");
                } else if (rnd < mRandomWaitProbA + mRandomWaitProbB) {
                    al::startAction(this, "WaitRandomB");
                }
            }
        }
    }

    if (isNearPlayerOrCap(this, mFindDistance)) {
        if (mRunawayCount > mRunawayCounter) {
            al::invalidateClipping(this);
            calcRunawayDir(&mRunawayDir, &mIsLeftRight, this);
            al::setNerve(this, &NrvSquirrel.Turn);
        } else {
            al::setVelocityZero(this);
            al::setNerve(this, &NrvSquirrel.Disappear);
        }
    }
}

// NON_MATCHING: cross product/dot product scheduling differs; regswap
void Squirrel::exeTurn() {
    if (al::isFirstStep(this)) {
        al::onCollide(this);
        if (mIsTurnOnPoint)
            al::setVelocityZero(this);

        if (mIsExistTurnLR) {
            sead::Vector3f frontDir = {0.0f, 0.0f, 0.0f};
            sead::Vector3f downDir = {0.0f, 0.0f, 0.0f};
            al::calcFrontDir(&frontDir, this);
            al::calcDownDir(&downDir, this);

            sead::Vector3f cross;
            cross.setCross(downDir, frontDir);

            if (al::isNearZero(cross, 0.001f) ||
                cross.x * mRunawayDir.x + cross.y * mRunawayDir.y +
                        cross.z * mRunawayDir.z <
                    0.0f)
                al::startAction(this, "TurnL");
            else
                al::startAction(this, "TurnR");
        } else {
            al::startAction(this, "Turn");
        }

        if (al::isExistShadowMaskCtrl(this)) {
            sead::Vector3f shadowDir = {0.0f, 0.0f, 0.0f};
            al::calcDownDir(&shadowDir, this);
            al::setShadowMaskDropDir(this, shadowDir);
        }
    }

    if (al::isVelocitySlow(this, 0.1f) || mIsTurnOnPoint) {
        sead::Vector3f downDir = {0.0f, 0.0f, 0.0f};
        al::calcDownDir(&downDir, this);
        al::addVelocityToDirection(this, downDir, 10.0f);
        al::scaleVelocity(this, mFriction);
        al::limitVelocityDirSign(this, downDir, 10.0f);
    } else {
        updateRunawayMove(this, &mFindDistance);
    }

    if (al::turnToDirection(this, mRunawayDir, mTurnSpeed)) {
        al::validateClipping(this);
        al::setNerve(this, &Runaway);
    }
}

// NON_MATCHING: Vector3f copy pattern (64+32 vs 3×32 bit)
void Squirrel::exeRunaway() {
    if (al::isFirstStep(this))
        al::startAction(this, "Move");

    updateRunawayMove(this, &mFindDistance);

    if (al::isCollidedWallVelocity(this)) {
        ++mRunawayCounter;
        if (!mIsValidReflectWall) {
            al::setVelocityZero(this);
            al::setNerve(this, &NrvSquirrel.Disappear);
            return;
        }

        const sead::Vector3f& wallNormal = al::getCollidedWallNormal(this);
        sead::Vector3f normal = wallNormal;
        sead::Vector3f downDir = {0.0f, 0.0f, 0.0f};
        al::calcDownDir(&downDir, this);
        al::verticalizeVec(&normal, downDir, normal);

        if (al::tryNormalizeOrZero(&normal)) {
            sead::Vector3f frontDir = {0.0f, 0.0f, 0.0f};
            al::calcFrontDir(&frontDir, this);
            al::verticalizeVec(&mRunawayDir, normal, frontDir);
            if (al::tryNormalizeOrZero(&mRunawayDir)) {
                al::turnVecToVecDegree(&mRunawayDir, mRunawayDir, normal, 15.0f);
                al::normalize(&mRunawayDir);
            } else {
                mRunawayDir = normal;
            }
            al::setNerve(this, &NrvSquirrel.Turn);
        } else {
            al::setVelocityZero(this);
            al::setNerve(this, &NrvSquirrel.Disappear);
        }
        return;
    }

    if (al::isCollidedGround(this)) {
        if (!al::isGreaterEqualStep(this, mMoveStep))
            return;
        if (!al::isOnGround(this, 0))
            return;

        ++mRunawayCounter;
        if (isNearPlayerOrCap(this, mFindDistance) && mRunawayCounter < mRunawayCount) {
            calcRunawayDir(&mRunawayDir, &mIsLeftRight, this);
            al::setNerve(this, &NrvSquirrel.Turn);
        } else {
            al::attachMtxConnectorToCollisionParts(mConnector,
                                                   al::getCollidedGroundCollisionParts(this));
            al::setConnectorBaseQuatTrans(al::getQuat(this), al::getTrans(this), mConnector);
            al::offCollide(this);
            al::setVelocityZero(this);
            al::setNerve(this, &NrvSquirrel.Wait);
        }
        return;
    }

    al::setVelocityZero(this);
    al::setNerve(this, &NrvSquirrel.Disappear);
}

void Squirrel::exeDisappear() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Disappear");
        al::setVelocityZero(this);
        al::offCollide(this);
    }

    if (al::isActionEnd(this)) {
        mItemGenerator->generate(al::getTrans(this), al::getQuat(this));

        if (mItemGenerator->isShine()) {
            kill();
            return;
        }

        mRunawayCounter = 0;
        al::offCollide(this);
        al::hideModelIfShow(this);
        al::invalidateHitSensors(this);
        al::invalidateClipping(this);
        al::startHitReaction(this, u8"消滅");
        al::setVelocityZero(this);
        al::resetQuatPosition(this, mInitQuat, mInitTrans);
        al::setConnectorBaseQuatTrans(al::getQuat(this), al::getTrans(this), mConnector);
        al::setNerve(this, &RestartPrepare);
    }
}

void Squirrel::exeRestartPrepare() {
    if (al::isFirstStep(this))
        al::startAction(this, al::getModelName(this));

    if (al::isJudgedToClipFrustum(this, 300.0f, 300.0f)) {
        if (++mRestartFrame >= 450) {
            al::validateHitSensors(this);
            al::validateClipping(this);
            if (mIsHide) {
                al::setNerve(this, &NrvSquirrel.Hide);
            } else {
                _12c = true;
                al::showModelIfHide(this);
                al::setNerve(this, &NrvSquirrel.Wait);
            }
        }
    }
}
