#include "Npc/TestAndoNpcIk.h"

#include <cmath>
#include <math/seadQuatCalcCommon.h>
#include <prim/seadDelegate.h>

#include "Library/Base/StringUtil.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"

#include "Npc/CyclicCoordinateDescentIk.h"
#include "Npc/SkeletonDynamics.h"
#include "Util/PlayerUtil.h"

static void quatToEulerDegrees(sead::Vector3f* out, const sead::Quatf* initQ,
                               const sead::Quatf* localQ) {
    sead::Quatf inv;
    inv.setInverse(*initQ);

    sead::Quatf relQ;
    relQ.setMul(inv, *localQ);

    relQ.calcRPY(*out);
    out->x *= 57.296f;
    out->y *= 57.296f;
    out->z *= 57.296f;
}

TestAndoNpcIk::TestAndoNpcIk(const char* name) : al::LiveActor(name) {}

TestAndoNpcIk::MyJoint* TestAndoNpcIk::createJoint(const char* jointName) {
    MyJoint* joint = new MyJoint();
    joint->name = jointName;
    al::calcJointPos(&joint->worldPos, this, jointName);
    al::calcJointQuat(&joint->worldQ, this, jointName);
    return joint;
}

void TestAndoNpcIk::linkJoint(MyJoint* parentJoint, MyJoint* childJoint) {
    parentJoint->child = childJoint;
    childJoint->parent = parentJoint;
}

void TestAndoNpcIk::createJointCtrl(MyJoint* joint) {
    al::initJointLocalZRotator(this, &joint->localRotZ, joint->name);
    al::initJointLocalYRotator(this, &joint->localRotY, joint->name);
    al::initJointLocalXRotator(this, &joint->localRotX, joint->name);
    joint->hasJointCtrl = true;
}

// NON_MATCHING: createJoint is inlined with different field init ordering
void TestAndoNpcIk::init(const al::ActorInitInfo& initInfo) {
    al::initActorWithArchiveName(this, initInfo, "CityWoman", nullptr);
    al::initJointControllerKeeper(this, 30);
    makeActorAlive();

    // Left arm chain
    MyJoint* shoulderL = createJoint("ShoulderL");
    MyJoint* armL1 = createJoint("ArmL1");
    MyJoint* armL2 = createJoint("ArmL2");
    MyJoint* handL = createJoint("HandL");
    MyJoint* fingerL = createJoint("FingerL");

    mLeftArmRoot = shoulderL;
    mLeftArmTip = fingerL;

    linkJoint(shoulderL, armL1);
    linkJoint(armL1, armL2);
    linkJoint(armL2, handL);
    linkJoint(handL, fingerL);

    createJointCtrl(armL1);
    createJointCtrl(armL2);
    createJointCtrl(handL);

    armL1->minAngle = {-45, -90, -80};
    armL1->maxAngle = {30, 60, 45};

    armL2->minAngle = {-90, 0, 0};
    armL2->maxAngle = {90, 90, 0};

    handL->minAngle = {0, 0, -45};
    handL->maxAngle = {0, 30, 45};

    // Right arm chain
    MyJoint* shoulderR = createJoint("ShoulderR");
    MyJoint* armR1 = createJoint("ArmR1");
    MyJoint* armR2 = createJoint("ArmR2");
    MyJoint* handR = createJoint("HandR");
    MyJoint* fingerR = createJoint("FingerR");

    mRightArmRoot = shoulderR;
    mRightArmTip = fingerR;

    linkJoint(shoulderR, armR1);
    linkJoint(armR1, armR2);
    linkJoint(armR2, handR);
    linkJoint(handR, fingerR);

    createJointCtrl(armR1);
    createJointCtrl(armR2);
    createJointCtrl(handR);

    armR1->minAngle = {-30, -60, -80};
    armR1->maxAngle = {45, 90, 45};

    armR2->minAngle = {-90, -90, 0};
    armR2->maxAngle = {90, 0, 0};

    handR->minAngle = {0, -30, -45};
    handR->maxAngle = {0, 0, 45};

    // Leg chain
    MyJoint* hip = createJoint("Hip");
    MyJoint* legR1 = createJoint("LegR1");
    MyJoint* legR2 = createJoint("LegR2");
    MyJoint* footR = createJoint("FootR");

    mLegRoot = hip;
    mLegTip = footR;

    linkJoint(hip, legR1);
    linkJoint(legR1, legR2);
    linkJoint(legR2, footR);

    createJointCtrl(legR1);
    createJointCtrl(legR2);

    legR1->minAngle = {-10, -30, -20};
    legR1->maxAngle = {45, 130, 90};

    legR2->minAngle = {0, -130, 0};
    legR2->maxAngle = {0, 0, 0};

    mIk = new CyclicCoordinateDescentIk(this, 1);
    mIk->createConnection("Hip", "FootL");
    mIk->setLimitDegree("LegL1", sead::Vector3f{-45, -130, 0},
                         sead::Vector3f{10, 30, 0}, "FootL");
    mIk->setLimitDegree("LegL2", sead::Vector3f{0, 0, 0},
                         sead::Vector3f{130, 0, 0}, "FootL");
}

// NON_MATCHING: regswap in Quatf copy (64-bit ldp vs 32-bit ldp)
void TestAndoNpcIk::initAfterPlacement() {
    calcLocalPose(mLeftArmRoot);
    calcLocalPose(mRightArmRoot);
    calcLocalPose(mLegRoot);

    for (MyJoint* j = mLeftArmRoot; j; j = j->child)
        j->initLocalQ = j->localQ;
    for (MyJoint* j = mRightArmRoot; j; j = j->child)
        j->initLocalQ = j->localQ;
    for (MyJoint* j = mLegRoot; j; j = j->child)
        j->initLocalQ = j->localQ;
}

// NON_MATCHING: regswap in rotate inline and setInverse
void TestAndoNpcIk::calcLocalPose(MyJoint* root) {
    for (MyJoint* j = root; j; j = j->child) {
        if (j->parent) {
            sead::Quatf pq = j->parent->worldQ;

            sead::Quatf invPQ;
            invPQ.setInverse(pq);

            j->localQ.setMul(invPQ, j->worldQ);
            j->localQ.normalize();

            sead::Vector3f diff = j->worldPos - j->parent->worldPos;
            diff.rotate(invPQ);
            j->localTranslation = diff;
        } else {
            j->localQ = j->worldQ;
            j->localTranslation = j->worldPos;
        }
    }
}

// NON_MATCHING: regswap in Quatf copy (64-bit ldp vs 32-bit ldp)
void TestAndoNpcIk::setInitQ(MyJoint* root) {
    for (MyJoint* j = root; j; j = j->child)
        j->initLocalQ = j->localQ;
}

// NON_MATCHING: regswap in rotate inlines and scheduling differences
void TestAndoNpcIk::control() {
    sead::Quatf actorQ;
    al::calcQuat(&actorQ, this);

    sead::Vector3f forward = sead::Vector3f::ez;
    forward.rotate(actorQ);

    sead::Vector3f shoulderPos;
    al::calcJointPos(&shoulderPos, this, "ShoulderR");
    shoulderPos.x += forward.x * 60.0f;
    shoulderPos.y += forward.y * 60.0f;
    shoulderPos.z += forward.z * 60.0f;

    sead::Quatf zRot;
    al::makeQuatZDegree(&zRot, (f32)mFrameCounter * 360.0f / 80.0f);

    sead::Quatf combinedQ;
    combinedQ.setMul(actorQ, zRot);

    sead::Vector3f up = sead::Vector3f::ey;
    up *= 80.0f;
    up.rotate(combinedQ);

    shoulderPos += up;
    shoulderPos.x -= sead::Vector3f::ey.x * 50.0f;
    shoulderPos.y -= sead::Vector3f::ey.y * 50.0f;
    shoulderPos.z -= sead::Vector3f::ey.z * 50.0f;

    mFrameCounter++;

    mIk->updateEffector(rs::getPlayerPos(this), 20.0f, "FootL");

    calcCCD(mLeftArmTip, mLeftArmRoot, rs::getPlayerHeadPos(this));
    calcLocalPose(mLeftArmRoot->child);

    calcCCD(mRightArmTip, mRightArmRoot, shoulderPos);
    calcLocalPose(mRightArmRoot->child);

    calcCCD(mLegTip, mLegRoot, rs::getPlayerPos(this));
    calcLocalPose(mLegRoot->child);

    for (MyJoint* j = mLeftArmRoot; j; j = j->child) {
        if (j->hasJointCtrl)
            quatToEulerDegrees((sead::Vector3f*)&j->localRotX, &j->initLocalQ,
                               &j->localQ);
    }
    for (MyJoint* j = mRightArmRoot; j; j = j->child) {
        if (j->hasJointCtrl)
            quatToEulerDegrees((sead::Vector3f*)&j->localRotX, &j->initLocalQ,
                               &j->localQ);
    }
    for (MyJoint* j = mLegRoot; j; j = j->child) {
        if (j->hasJointCtrl)
            quatToEulerDegrees((sead::Vector3f*)&j->localRotX, &j->initLocalQ,
                               &j->localQ);
    }
}

// NON_MATCHING: regswap throughout rotate and setMul inlines
void TestAndoNpcIk::calcCCD(MyJoint* tip, MyJoint* root,
                             const sead::Vector3f& target) {
    sead::Vector3f goalDir;
    goalDir.x = target.x - tip->worldPos.x;
    goalDir.y = target.y - tip->worldPos.y;
    goalDir.z = target.z - tip->worldPos.z;

    f32 distSq = goalDir.x * goalDir.x + goalDir.y * goalDir.y +
                 goalDir.z * goalDir.z;
    if (distSq > 400.0f) {
        f32 dist = sqrtf(distSq);
        if (dist > 0.0f) {
            f32 scale = 20.0f / dist;
            goalDir.x *= scale;
            goalDir.y *= scale;
            goalDir.z *= scale;
        }
    }

    sead::Vector3f goalPos;
    goalPos.x = goalDir.x + tip->worldPos.x;
    goalPos.y = goalDir.y + tip->worldPos.y;
    goalPos.z = goalDir.z + tip->worldPos.z;

    for (u32 iter = 0; iter < 10; iter++) {
        MyJoint* j = tip->parent;
        while (j != root) {
            sead::Vector3f toGoal = goalPos - j->worldPos;
            if (!al::tryNormalizeOrZero(&toGoal))
                return;

            sead::Vector3f toTip;
            toTip.x = tip->worldPos.x - j->worldPos.x;
            toTip.y = tip->worldPos.y - j->worldPos.y;
            toTip.z = tip->worldPos.z - j->worldPos.z;
            if (!al::tryNormalizeOrZero(&toTip)) {
                j = j->parent;
                continue;
            }

            sead::Quatf rotQ;
            al::makeQuatRotationRate(&rotQ, toTip, toGoal, 1.0f);

            sead::Quatf newQ;
            newQ.setMul(rotQ, j->worldQ);
            newQ.normalize();

            // Compute reference quaternion
            sead::Quatf refQ = sead::Quatf::unit;
            if (j->parent) {
                refQ.setMul(j->parent->worldQ, j->initLocalQ);
                refQ.normalize();
            }

            sead::Vector3f euler;
            quatToEulerDegrees(&euler, &refQ, &newQ);

            if (euler.x < j->minAngle.x)
                euler.x = j->minAngle.x;
            if (euler.y < j->minAngle.y)
                euler.y = j->minAngle.y;
            if (euler.z < j->minAngle.z)
                euler.z = j->minAngle.z;
            if (euler.x > j->maxAngle.x)
                euler.x = j->maxAngle.x;
            if (euler.y > j->maxAngle.y)
                euler.y = j->maxAngle.y;
            if (euler.z > j->maxAngle.z)
                euler.z = j->maxAngle.z;

            euler.x *= 0.95f;
            f32 halfX = euler.x * 0.017453f * 0.5f;
            f32 halfY = euler.y * 0.017453f * 0.5f;
            f32 halfZ = euler.z * 0.017453f * 0.5f;

            f32 cz = cosf(halfZ);
            f32 cy = cosf(halfY);
            f32 cx = cosf(halfX);
            f32 sz = sinf(halfZ);
            f32 sy = sinf(halfY);
            f32 sx = sinf(halfX);

            sead::Quatf eulerQ;
            eulerQ.x = cx * cy * sx - cz * sy * sz;
            eulerQ.w = cx * cy * cz + sy * sz * sx;
            eulerQ.y = cz * cx * sy + cy * sz * sx;
            eulerQ.z = cz * cy * sz - cx * sy * sx;

            sead::Quatf finalQ;
            finalQ.setMul(refQ, eulerQ);
            finalQ.normalize();

            // Compute delta: finalQ * worldQ^{-1}
            j->worldQ.normalize();
            newQ.setInverse(j->worldQ);
            refQ.setMul(finalQ, newQ);
            refQ.normalize();

            rotateJoint(j, j, refQ);

            if (al::isNear(j->worldPos, goalPos, 0.001f))
                return;

            j = j->parent;
        }
    }
}

void TestAndoNpcIk::applyModel(MyJoint* root) {
    for (MyJoint* j = root; j; j = j->child) {
        if (j->hasJointCtrl)
            quatToEulerDegrees((sead::Vector3f*)&j->localRotX, &j->initLocalQ,
                               &j->localQ);
    }
}

TestAndoNpcIk::MyJoint* TestAndoNpcIk::createEmptyJoint(const char* jointName) {
    MyJoint* joint = new MyJoint();
    joint->name = jointName;
    return joint;
}

// NON_MATCHING: regswap in rotate inline
void TestAndoNpcIk::rotateJoint(MyJoint* start, MyJoint* pivot,
                                 const sead::Quatf& rot) {
    for (MyJoint* j = start; j; j = j->child) {
        sead::Quatf newQ;
        newQ.setMul(rot, j->worldQ);
        newQ.normalize();
        j->worldQ = newQ;

        sead::Vector3f diff;
        diff.x = j->worldPos.x - pivot->worldPos.x;
        diff.y = j->worldPos.y - pivot->worldPos.y;
        diff.z = j->worldPos.z - pivot->worldPos.z;

        diff.rotate(rot);

        j->worldPos.x = pivot->worldPos.x + diff.x;
        j->worldPos.y = pivot->worldPos.y + diff.y;
        j->worldPos.z = pivot->worldPos.z + diff.z;
    }
}

void TestAndoNpcIk::extendBone(MyJoint* root, f32 scale) {
    for (MyJoint* j = root; j; j = j->child) {
        j->localTranslation.x *= scale;
        j->localTranslation.y *= scale;
        j->localTranslation.z *= scale;
    }
}

// NON_MATCHING: regswap in rotate inline and Quatf copy width
void TestAndoNpcIk::backLocalPose(MyJoint* root) {
    for (MyJoint* j = root; j; j = j->child) {
        if (j->parent) {
            sead::Quatf newWorldQ;
            newWorldQ.setMul(j->parent->worldQ, j->localQ);
            newWorldQ.normalize();
            j->worldQ = newWorldQ;

            sead::Vector3f t = j->localTranslation;
            t.rotate(j->parent->worldQ);

            j->worldPos.x = j->parent->worldPos.x + t.x;
            j->worldPos.y = j->parent->worldPos.y + t.y;
            j->worldPos.z = j->parent->worldPos.z + t.z;
        } else {
            j->worldQ = j->localQ;
            j->worldPos = j->localTranslation;
        }
    }
}

// TestAndoNpcIk2

TestAndoNpcIk2::TestAndoNpcIk2(const char* name) : al::LiveActor(name) {}

void TestAndoNpcIk2::init(const al::ActorInitInfo& initInfo) {
    al::initActorWithArchiveName(this, initInfo, "CityWoman", nullptr);
    mSkeletonDynamics = new SkeletonDynamics(this);

    auto* delegate =
        new sead::Delegate1<TestAndoNpcIk2, SkeletonDynamicsCallbackInfo*>(
            this, &TestAndoNpcIk2::callback);
    mSkeletonDynamics->setDelegate(delegate);
    makeActorAlive();
    al::startAction(this, "Walk");
    mSkeletonDynamics->init();
}

// NON_MATCHING: regswap in rotate inline
void TestAndoNpcIk2::callback(SkeletonDynamicsCallbackInfo* info) {
    if (!al::isEqualString(info->name, "HandL"))
        return;

    sead::Vector3f toPlayer;
    const sead::Vector3f& headPos = rs::getPlayerHeadPos(this);
    toPlayer.x = headPos.x - info->pos.x;
    toPlayer.y = headPos.y - info->pos.y;
    toPlayer.z = headPos.z - info->pos.z;

    f32 distSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y +
                 toPlayer.z * toPlayer.z;
    if (sqrtf(distSq) > 300.0f) {
        f32 dist = sqrtf(distSq);
        if (dist > 0.0f) {
            f32 scale = 300.0f / dist;
            toPlayer.x *= scale;
            toPlayer.y *= scale;
            toPlayer.z *= scale;
        }
    }

    info->pos.x = toPlayer.x + info->pos.x;
    info->pos.y = toPlayer.y + info->pos.y;
    info->pos.z = toPlayer.z + info->pos.z;

    sead::Vector3f dir = {toPlayer.x, toPlayer.y, toPlayer.z};
    if (!al::tryNormalizeOrZero(&dir))
        return;

    sead::Quatf q = info->quat;
    sead::Vector3f curDir = sead::Vector3f::ez;
    curDir.rotate(q);

    sead::Quatf rotQ;
    al::makeQuatRotationRate(&rotQ, curDir, dir, 1.0f);

    sead::Quatf newQ;
    newQ.setMul(rotQ, info->quat);
    newQ.normalize();
    info->quat = newQ;
}

void TestAndoNpcIk2::calcAnim() {
    al::LiveActor::calcAnim();
    mSkeletonDynamics->update();
}
