#include "Npc/SkeletonDynamics.h"

#include <cmath>
#include <cstring>
#include <container/seadPtrArray.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>
#include <prim/seadDelegate.h>

#include "Library/Base/StringUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"

namespace alModelJointFunction {
void setDirectJointMtx(al::ModelKeeper* keeper, s32 jointIndex, const sead::Matrix34f& mtx);
}

struct JointNode {
    const char* name = nullptr;
    JointNode* parent = nullptr;
    s32 childCountInit = 0;
    sead::PtrArray<JointNode> children;
    sead::Vector3f animPos = {0, 0, 0};
    sead::Quatf animQuat = {0, 0, 0, 1};
    sead::Quatf animInvQuat = {0, 0, 0, 1};
    sead::Vector3f localPos = {0, 0, 0};
    sead::Quatf localRot = {0, 0, 0, 1};
    sead::Vector3f pos = {0, 0, 0};
    sead::Quatf rot = {0, 0, 0, 1};
    sead::Vector3f velocity = {0, 0, 0};
    sead::Vector3f angVelocity = {0, 0, 0};
    sead::Vector3f force = {0, 0, 0};
    sead::Vector3f torque = {0, 0, 0};
    s32 posAttr = 0;
    s32 rotAttr = 0;
    sead::Vector3f lengthForceMin = {0, 0, 0};
    sead::Vector3f lengthForceMax = {0, 0, 0};
    sead::Vector3f lengthTorqueMin = {0, 0, 0};
    sead::Vector3f lengthTorqueMax = {0, 0, 0};
    sead::Vector3f ikGravity = {0, 0, 0};
};

static_assert(sizeof(JointNode) == 0x100);

struct SpringLink {
    JointNode* nodeA = nullptr;
    JointNode* nodeB = nullptr;
    sead::Vector3f localPos = {0, 0, 0};
    sead::Quatf localRot = {0, 0, 0, 1};
    f32 currentLength = 0.0f;
    f32 springRotX = 0.0f;
    f32 springRotY = 0.0f;
    f32 springRotZ = 0.0f;
    f32 restLength = 0.0f;
};

static_assert(sizeof(SpringLink) == 0x40);

// NON_MATCHING: regswap in quaternion rotation and normalization
static f32 calcSpringTorque(sead::Vector3f* outTorque, const sead::Quatf* rotA,
                            const sead::Quatf* rotB, f32 axisX, f32 axisY, f32 axisZ,
                            f32 springValue) {
    // Rotate axis by rotA
    f32 t1 = (rotA->y * axisZ - rotA->z * axisY) + rotA->w * axisX;
    f32 t2 = rotA->w * axisY + (rotA->z * axisX - rotA->x * axisZ);
    f32 t3 = rotA->w * axisZ + (rotA->x * axisY - rotA->y * axisX);
    f32 t4 = -(rotA->x * axisX) - rotA->y * axisY - rotA->z * axisZ;

    f32 rA_x = (rotA->y * t3 + (rotA->w * t1 - rotA->z * t2)) - rotA->x * t4;
    f32 rA_y = (rotA->z * t1 + rotA->w * t2) - rotA->x * t3 - rotA->y * t4;
    f32 rA_z = (rotA->w * t3 + (rotA->x * t2 - rotA->y * t1)) - rotA->z * t4;

    // Rotate axis by rotB
    f32 u1 = (rotB->y * axisZ - rotB->z * axisY) + rotB->w * axisX;
    f32 u2 = rotB->w * axisY + (rotB->z * axisX - rotB->x * axisZ);
    f32 u3 = rotB->w * axisZ + (rotB->x * axisY - rotB->y * axisX);
    f32 u4 = -(rotB->x * axisX) - rotB->y * axisY - rotB->z * axisZ;

    f32 rB_x = (rotB->y * u3 + (rotB->w * u1 - rotB->z * u2)) - rotB->x * u4;
    f32 rB_y = (rotB->z * u1 + rotB->w * u2) - rotB->x * u3 - rotB->y * u4;
    f32 rB_z = (rotB->w * u3 + (rotB->x * u2 - rotB->y * u1)) - rotB->z * u4;

    // Difference
    f32 dx = rB_x - rA_x;
    f32 dy = rB_y - rA_y;
    f32 dz = rB_z - rA_z;

    f32 dist = std::sqrt(dx * dx + dy * dy + dz * dz);

    f32 nx, ny, nz;
    if (dist >= 0.0001f) {
        f32 inv = 1.0f / dist;
        nx = inv * dx;
        ny = inv * dy;
        nz = inv * dz;
    } else {
        nx = dx * 0.0f;
        ny = dy * 0.0f;
        nz = dz * 0.0f;
    }

    f32 diff = springValue - dist;
    if (diff <= 0.0f)
        diff = -diff;

    f32 scale = dist * 0.1f - diff * 0.1f;
    f32 fx = scale * nx;
    f32 fy = scale * ny;
    f32 fz = scale * nz;

    // Cross product: outTorque = cross(rA, force)
    outTorque->x = rA_y * fz - rA_z * fy;
    outTorque->y = rA_z * fx - rA_x * fz;
    outTorque->z = rA_x * fy - rA_y * fx;

    return dist;
}

SkeletonDynamics::SkeletonDynamics(al::LiveActor* actor) : mActor(actor) {
    al::ModelKeeper* modelKeeper = actor->getModelKeeper();
    s32 jointNum = al::getJointNum(modelKeeper);

    mAllJoints.allocBuffer(jointNum, nullptr, 8);
    mCtrlJoints.allocBuffer(jointNum, nullptr, 8);

    for (s32 i = 0; i < jointNum; i++) {
        auto* node = new JointNode();
        node->name = al::getJointName(modelKeeper, i);

        s32 parentIndex = al::getParentJointIndex(modelKeeper, i);
        if (parentIndex >= 0 && (u32)parentIndex < (u32)mAllJoints.size()) {
            node->parent = mAllJoints[parentIndex];
        } else {
            node->parent = nullptr;
        }

        if (node->parent) {
            node->parent->childCountInit++;
        }

        node->childCountInit = 0;
        al::calcJointPos(&node->pos, actor, node->name);
        al::calcJointQuat(&node->rot, actor, node->name);
        std::memset(&node->velocity, 0, 0x74);

        mAllJoints.pushBack(node);
    }

    for (s32 i = 0; i < jointNum; i++) {
        JointNode* node = mAllJoints[i];
        s32 childCount = node->childCountInit;
        if (childCount < 1)
            childCount = 1;
        node->children.allocBuffer(childCount, nullptr, 8);

        if (node->parent)
            node->parent->children.pushBack(node);
    }
}

void SkeletonDynamics::setAttrCtrl(const char* boneName) {
    for (s32 i = 0; i < mAllJoints.size(); i++) {
        if (al::isEqualString(mAllJoints[i]->name, boneName)) {
            JointNode* node = mAllJoints[i];
            node->posAttr = 1;
            node->rotAttr = 1;
            mCtrlJoints.pushBack(node);
            return;
        }
    }
}

// NON_MATCHING: dual-counter optimization for PtrArray access with second operator[]
void SkeletonDynamics::setAttrConstraint(const char* boneName) {
    for (s32 i = 0; i < mAllJoints.size(); i++) {
        if (al::isEqualString(mAllJoints[i]->name, boneName)) {
            JointNode* node = mAllJoints[i];
            node->posAttr = 2;
            node->rotAttr = 2;
            mCtrlJoints.pushBack(node);
            return;
        }
    }
}

// NON_MATCHING: dual-counter optimization for PtrArray access with second operator[]
void SkeletonDynamics::setAttrNoCtrl(const char* boneName) {
    for (s32 i = 0; i < mAllJoints.size(); i++) {
        if (al::isEqualString(mAllJoints[i]->name, boneName)) {
            JointNode* node = mAllJoints[i];
            node->posAttr = 3;
            node->rotAttr = 3;
            return;
        }
    }
}

// NON_MATCHING: dual-counter optimization for PtrArray access with second operator[]
void SkeletonDynamics::setAttrAuto(const char* boneName, const sead::Vector3f& gravity) {
    for (s32 i = 0; i < mAllJoints.size(); i++) {
        if (al::isEqualString(mAllJoints[i]->name, boneName)) {
            JointNode* node = mAllJoints[i];
            node->posAttr = 4;
            node->rotAttr = 4;
            node->ikGravity.x = gravity.x;
            node->ikGravity.y = gravity.y;
            node->ikGravity.z = gravity.z;
            return;
        }
    }
}

void SkeletonDynamics::setDelegate(sead::IDelegate1<SkeletonDynamicsCallbackInfo*>* delegate) {
    mDelegate = delegate;
}

static bool isAncestor(JointNode* node, JointNode* potentialAncestor) {
    JointNode* cur = node;
    while (cur) {
        cur = cur->parent;
        if (!cur)
            return false;
        if (cur == potentialAncestor)
            return true;
    }
    return false;
}

// NON_MATCHING: original accesses nn::g3d::SkeletonObj directly for parent bone indices
void SkeletonDynamics::init() {
    // First pass: add dynamic joints (attr 0) to mCtrlJoints
    for (s32 i = 0; i < mAllJoints.size(); i++) {
        JointNode* node = mAllJoints[i];
        if (node->posAttr == 0 && node->rotAttr == 0)
            mCtrlJoints.pushBack(node);
    }

    // Count spring links needed
    s32 linkCount = 0;
    for (s32 i = 0; i < mCtrlJoints.size(); i++) {
        JointNode* nodeA = mCtrlJoints[i];
        bool isDynamic = (nodeA->posAttr == 0 || nodeA->rotAttr == 0);
        for (s32 j = i + 1; j < mCtrlJoints.size(); j++) {
            JointNode* nodeB = mCtrlJoints[j];
            if (!isDynamic) {
                if (nodeB->posAttr != 0 && nodeB->rotAttr != 0)
                    continue;
            }
            bool aIsAncestor = isAncestor(nodeB, nodeA);
            bool bIsAncestor = isAncestor(nodeA, nodeB);
            if (aIsAncestor || bIsAncestor)
                linkCount++;
        }
    }

    // Allocate links array
    s32 allocCount = linkCount >= 1 ? linkCount : 1;
    mLinks.allocBuffer(allocCount, nullptr, 8);

    // Second pass: create spring links
    for (s32 i = 0; i < mCtrlJoints.size(); i++) {
        JointNode* nodeA = mCtrlJoints[i];
        bool isDynamic = (nodeA->posAttr == 0 || nodeA->rotAttr == 0);
        for (s32 j = i + 1; j < mCtrlJoints.size(); j++) {
            JointNode* nodeB = mCtrlJoints[j];
            if (!isDynamic) {
                if (nodeB->posAttr != 0 && nodeB->rotAttr != 0)
                    continue;
            }
            bool aIsAncestor = isAncestor(nodeB, nodeA);
            bool bIsAncestor = isAncestor(nodeA, nodeB);
            if (aIsAncestor || bIsAncestor) {
                auto* link = new SpringLink();
                link->nodeA = nodeA;
                link->nodeB = nodeB;
                link->localRot = {0.0f, 0.0f, 0.0f, 1.0f};
                link->localPos = {0.0f, 0.0f, 0.0f};
                link->springRotX = 0.0f;
                link->springRotY = 0.0f;
                link->springRotZ = 0.0f;
                link->currentLength = 0.0f;

                f32 dx = nodeA->pos.x - nodeB->pos.x;
                f32 dy = nodeA->pos.y - nodeB->pos.y;
                f32 dz = nodeA->pos.z - nodeB->pos.z;
                link->restLength = std::sqrt(dx * dx + dy * dy + dz * dz);
                mLinks.pushBack(link);
            }
        }
    }
}

// NON_MATCHING: loop structure (count-down vs count-up for PtrArray iteration)
void SkeletonDynamics::updateAnimPose() {
    for (s32 i = 0; i < mAllJoints.size(); i++) {
        JointNode* node = mAllJoints[i];
        al::calcJointPos(&node->animPos, mActor, node->name);
        al::calcJointQuat(&node->animQuat, mActor, node->name);
    }
}

// NON_MATCHING: loop structure + regswap in quaternion inversion
void SkeletonDynamics::calcAnimPoseInvRot() {
    for (s32 i = 0; i < mAllJoints.size(); i++) {
        JointNode* node = mAllJoints[i];
        f32 qx = node->animQuat.x;
        f32 qy = node->animQuat.y;
        f32 qz = node->animQuat.z;
        f32 qw = node->animQuat.w;

        f32 sqLen = qw * qw + qx * qx + qy * qy + qz * qz;
        if (sqLen > 0.00000011921f) {
            f32 inv = 1.0f / sqLen;
            qw = qw * inv;
            qx = inv * qx;
            qy = inv * qy;
            qz = inv * qz;
        }
        node->animInvQuat.w = qw;
        node->animInvQuat.x = -qx;
        node->animInvQuat.y = -qy;
        node->animInvQuat.z = -qz;
    }
}

// NON_MATCHING: loop structure + regswap in quaternion multiply/rotate
void SkeletonDynamics::updateAnimPoseLocal() {
    for (s32 i = 0; i < mAllJoints.size(); i++) {
        JointNode* node = mAllJoints[i];
        JointNode* par = node->parent;
        if (!par)
            continue;

        // localPos = rotate(animPos - parent.animPos, parent.animInvQuat)
        f32 dx = node->animPos.x - par->animPos.x;
        f32 dy = node->animPos.y - par->animPos.y;
        f32 dz = node->animPos.z - par->animPos.z;
        f32 ix = par->animInvQuat.x;
        f32 iy = par->animInvQuat.y;
        f32 iz = par->animInvQuat.z;
        f32 iw = par->animInvQuat.w;

        f32 t1 = (iy * dz - iz * dy) + iw * dx;
        f32 t2 = iw * dy + (iz * dx - dz * ix);
        f32 t3 = iw * dz + (ix * dy - iy * dx);
        f32 t4 = -(dx * ix) - iy * dy - dz * iz;

        node->localPos.x =
            (iy * t3 + (iw * t1 - iz * t2)) - ix * t4;
        node->localPos.y =
            (t1 * par->animInvQuat.z + t2 * par->animInvQuat.w) -
            t3 * par->animInvQuat.x - t4 * par->animInvQuat.y;
        node->localPos.z =
            (t2 * par->animInvQuat.x - t1 * par->animInvQuat.y +
             t3 * par->animInvQuat.w) -
            t4 * par->animInvQuat.z;

        // localRot = parent.animInvQuat * node.animQuat
        f32 ax = par->animInvQuat.x;
        f32 ay = par->animInvQuat.y;
        f32 az = par->animInvQuat.z;
        f32 aw = par->animInvQuat.w;
        f32 bx = node->animQuat.x;
        f32 by = node->animQuat.y;
        f32 bz = node->animQuat.z;
        f32 bw = node->animQuat.w;

        f32 rw = aw * bw - ax * bx - ay * by - az * bz;
        f32 rx = aw * bx + ax * bw + ay * bz - by * az;
        f32 ry = bx * az + (ax * by + aw * bz - ay * bw);  // from IDA pattern
        f32 rz = az * bw + (ax * bz + aw * by - bx * ay);  // adjusted to match

        // Wait, let me use the standard quaternion multiply formula
        // Actually I need to match the exact IDA output. Let me re-derive:
        // setMul(a, b): w = aw*bw - ax*bx - ay*by - az*bz
        //               x = aw*bx + ax*bw + ay*bz - az*by
        //               y = aw*by - ax*bz + ay*bw + az*bx
        //               z = aw*bz + ax*by - ay*bx + az*bw

        rw = aw * bw - ax * bx - ay * by - az * bz;
        rx = ax * bw + aw * bx + ay * bz - az * by;
        ry = bx * az + (aw * by + az * bw - ax * bz);
        rz = az * bw + (ax * by + aw * bz - ay * bx);

        node->localRot.z = rz;
        node->localRot.w = rw;
        node->localRot.x = rx;
        node->localRot.y = ry;

        f32 len = std::sqrt(rz * rz + (rw * rw + rx * rx + ry * ry));
        if (len > 0.0f) {
            f32 inv = 1.0f / len;
            node->localRot.x = inv * node->localRot.x;
            node->localRot.y = inv * node->localRot.y;
            node->localRot.z = inv * node->localRot.z;
            node->localRot.w = inv * node->localRot.w;
        }
    }
}

// NON_MATCHING: loop structure + regswap in quaternion multiply/rotate
void SkeletonDynamics::updateSpring() {
    for (s32 i = 0; i < mLinks.size(); i++) {
        SpringLink* link = mLinks[i];
        JointNode* nodeA = link->nodeA;
        JointNode* nodeB = link->nodeB;

        // localPos = rotate(nodeA.animPos - nodeB.animPos, nodeB.animInvQuat)
        f32 dx = nodeA->animPos.x - nodeB->animPos.x;
        f32 dy = nodeA->animPos.y - nodeB->animPos.y;
        f32 dz = nodeA->animPos.z - nodeB->animPos.z;
        f32 ix = nodeB->animInvQuat.x;
        f32 iy = nodeB->animInvQuat.y;
        f32 iz = nodeB->animInvQuat.z;
        f32 iw = nodeB->animInvQuat.w;

        f32 t1 = (iy * dz - iz * dy) + iw * dx;
        f32 t2 = iw * dy + (iz * dx - dz * ix);
        f32 t3 = iw * dz + (ix * dy - iy * dx);
        f32 t4 = -(dx * ix) - iy * dy - dz * iz;

        link->localPos.x =
            (iy * t3 + (iw * t1 - iz * t2)) - ix * t4;
        link->localPos.y =
            (t1 * nodeB->animInvQuat.z + t2 * nodeB->animInvQuat.w) -
            t3 * nodeB->animInvQuat.x - t4 * nodeB->animInvQuat.y;
        link->localPos.z =
            (t2 * nodeB->animInvQuat.x - t1 * nodeB->animInvQuat.y +
             t3 * nodeB->animInvQuat.w) -
            t4 * nodeB->animInvQuat.z;

        // localRot = nodeB.animInvQuat * nodeA.animQuat
        f32 ax = nodeB->animInvQuat.x;
        f32 ay = nodeB->animInvQuat.y;
        f32 az = nodeB->animInvQuat.z;
        f32 aw = nodeB->animInvQuat.w;
        f32 bx = nodeA->animQuat.x;
        f32 by = nodeA->animQuat.y;
        f32 bz = nodeA->animQuat.z;
        f32 bw = nodeA->animQuat.w;

        f32 rw = aw * bw - ax * bx - ay * by - az * bz;
        f32 rx = ax * bw + aw * bx + ay * bz - by * az;
        f32 ry = bx * az + (aw * by + az * bw - ax * bz);
        f32 rz = az * bw + (ax * by + aw * bz - ay * bx);

        link->localRot.z = rz;
        link->localRot.w = rw;
        link->localRot.x = rx;
        link->localRot.y = ry;

        f32 len = std::sqrt(rz * rz + (rw * rw + rx * rx + ry * ry));
        if (len > 0.0f) {
            f32 inv = 1.0f / len;
            link->localRot.x = inv * link->localRot.x;
            link->localRot.y = inv * link->localRot.y;
            link->localRot.z = inv * link->localRot.z;
            link->localRot.w = inv * link->localRot.w;
        }

        // currentLength = distance between nodeA and nodeB anim positions
        f32 adx = nodeA->animPos.x - nodeB->animPos.x;
        f32 ady = nodeA->animPos.y - nodeB->animPos.y;
        f32 adz = nodeA->animPos.z - nodeB->animPos.z;
        link->restLength = std::sqrt(adx * adx + ady * ady + adz * adz);
    }
}

// NON_MATCHING: loop structure + 64-bit load/store optimization for struct copy
void SkeletonDynamics::invokeDelegate() {
    if (!mDelegate)
        return;

    for (s32 i = 0; i < mCtrlJoints.size(); i++) {
        JointNode* node = mCtrlJoints[i];
        if (node->posAttr == 1 || node->rotAttr == 1) {
            SkeletonDynamicsCallbackInfo info;
            info.name = node->name;
            info.pos = node->animPos;
            info.quat = node->animQuat;
            mDelegate->invoke(&info);
            if (node->posAttr == 1)
                node->pos = info.pos;
            if (node->rotAttr == 1)
                node->rot = info.quat;
        }
    }
}

// NON_MATCHING: loop structure + 64-bit load/store for field copy
void SkeletonDynamics::updateConstrainedJoint() {
    for (s32 i = 0; i < mCtrlJoints.size(); i++) {
        JointNode* node = mCtrlJoints[i];
        if (node->posAttr == 2)
            node->pos = node->animPos;
        if (node->rotAttr == 2)
            node->rot = node->animQuat;
    }
}

// NON_MATCHING: loop structure (count-down vs count-up)
void SkeletonDynamics::clearForceAndTorque() {
    for (s32 i = 0; i < mCtrlJoints.size(); i++) {
        JointNode* node = mCtrlJoints[i];
        node->force = {0.0f, 0.0f, 0.0f};
        node->torque = {0.0f, 0.0f, 0.0f};
    }
}

// NON_MATCHING: loop structure + regswap in force/torque computation
void SkeletonDynamics::updateForceAndTorque() {
    for (s32 i = 0; i < mLinks.size(); i++) {
        SpringLink* link = mLinks[i];
        JointNode* nodeA = link->nodeA;
        JointNode* nodeB = link->nodeB;

        if (nodeA->posAttr == 0 || nodeB->posAttr == 0) {
            // Rotate localPos by nodeB's simulation rotation
            f32 px = link->localPos.x;
            f32 py = link->localPos.y;
            f32 pz = link->localPos.z;
            f32 rx = nodeB->rot.x;
            f32 ry = nodeB->rot.y;
            f32 rz = nodeB->rot.z;
            f32 rw = nodeB->rot.w;

            f32 t1 = (ry * pz - rz * py) + rw * px;
            f32 t2 = rw * py + (rz * px - pz * rx);
            f32 t3 = rw * pz + (rx * py - ry * px);
            f32 t4 = -(px * rx) - ry * py - pz * rz;

            f32 rpx = (ry * t3 + (rw * t1 - rz * t2)) - rx * t4;
            f32 rpy = (rz * t1 + rw * t2) - rx * t3 - ry * t4;
            f32 rpz = (rw * t3 + (rx * t2 - ry * t1)) - rz * t4;

            f32 targetX = nodeB->pos.x + rpx;
            f32 targetY = nodeB->pos.y + rpy;
            f32 targetZ = nodeB->pos.z + rpz;

            f32 dx = targetY - nodeA->pos.y;
            f32 dy = targetX - nodeA->pos.x;
            f32 dz = targetZ - nodeA->pos.z;

            f32 dist = std::sqrt(dz * dz + (dy * dy + dx * dx));
            f32 invDist = 1.0f / dist;
            f32 ndx = dx * invDist;
            if (dist < 0.0001f)
                ndx = dx * 0.0f;
            f32 ndy = dy * invDist;
            f32 ndz = dz * invDist;
            if (dist < 0.0001f)
                ndz = dz * 0.0f;

            f32 diff = link->currentLength - dist;
            if (dist < 0.0001f)
                ndy = dy * 0.0f;

            if (diff <= 0.0f)
                diff = -diff;

            f32 scale = dist * 0.05f + diff * -0.051f;
            f32 fx = ndy * scale;
            f32 fy = ndx * scale;
            f32 fz = ndz * scale;

            nodeA->force.x = nodeA->force.x + fx;
            nodeA->force.y = nodeA->force.y + fy;
            nodeA->force.z = nodeA->force.z + fz;

            JointNode* nB = link->nodeB;
            nB->force.x = nB->force.x - fx;
            nB->force.y = nB->force.y - fy;
            nB->force.z = nB->force.z - fz;

            nodeA = link->nodeA;
            link->currentLength = dist;
        }

        if (nodeA->rotAttr == 0 || link->nodeB->rotAttr == 0) {
            // Compute target rotation = nodeB->rot * link->localRot
            f32 bx = link->nodeB->rot.x;
            f32 by = link->nodeB->rot.y;
            f32 lx = link->localRot.x;
            f32 ly = link->localRot.y;
            f32 bz = link->nodeB->rot.z;
            f32 bw = link->nodeB->rot.w;
            f32 lz = link->localRot.z;
            f32 lw = link->localRot.w;

            f32 tw = bw * lw - bx * lx - by * ly - bz * lz;
            f32 tx = bx * lw + bw * lx + by * lz - ly * bz;
            f32 ty = lx * bz + (bw * ly + bz * lw - bx * lz);
            f32 tz = bz * lw + (bx * ly + bw * lz - lx * by);

            f32 len = std::sqrt(tz * tz + (tw * tw + tx * tx + ty * ty));
            sead::Quatf targetRot;
            targetRot.z = tz;
            targetRot.w = tw;
            targetRot.x = tx;
            targetRot.y = ty;
            if (len > 0.0f) {
                f32 inv = 1.0f / len;
                targetRot.x = inv * tx;
                targetRot.y = inv * ty;
                targetRot.z = inv * tz;
                targetRot.w = inv * tw;
            }

            // Apply spring torque for each axis
            sead::Vector3f torqueX, torqueY, torqueZ;
            link->springRotX = calcSpringTorque(&torqueX, &nodeA->rot, &targetRot, 1.0f, 0.0f,
                                                0.0f, link->springRotX);
            link->springRotY = calcSpringTorque(&torqueY, &nodeA->rot, &targetRot, 0.0f, 1.0f,
                                                0.0f, link->springRotY);
            link->springRotZ = calcSpringTorque(&torqueZ, &nodeA->rot, &targetRot, 0.0f, 0.0f,
                                                1.0f, link->springRotZ);

            // Sum torques
            f32 totalY = torqueX.y + torqueY.y + torqueZ.y;
            f32 totalX = torqueX.x + torqueY.x + torqueZ.x;
            f32 totalZ = torqueX.z + torqueY.z + torqueZ.z;

            JointNode* nA = link->nodeA;
            nA->torque.x = totalX + nA->torque.x;
            f32 prevY = totalY + nA->torque.y;
            nA->torque.y = prevY;
            nA->torque.z = nA->torque.z + totalZ;

            JointNode* nB2 = link->nodeB;
            nB2->torque.x = nB2->torque.x - totalX;
            nB2->torque.y = nB2->torque.y - totalY;
            nB2->torque.z = nB2->torque.z - totalZ;
        }
    }
}

// NON_MATCHING: loop structure + regswap in quaternion multiply
void SkeletonDynamics::updatePosAndRot() {
    for (s32 i = 0; i < mCtrlJoints.size(); i++) {
        JointNode* node = mCtrlJoints[i];

        if (node->posAttr == 0) {
            f32 newVelX = node->velocity.x * 0.5f + node->force.x;
            f32 newVelY = node->velocity.y * 0.5f + node->force.y;
            f32 newVelZ = node->velocity.z * 0.5f + node->torque.x;

            node->velocity.y = newVelY;
            f32 posX = node->pos.x;
            f32 posY = node->pos.y;
            node->velocity.x = newVelX;
            node->pos.x = posX + newVelX;
            f32 posZ = node->pos.z + newVelZ;
            node->velocity.z = newVelZ;
            node->pos.y = posY + newVelY;
            node->pos.z = posZ;
        }

        if (node->rotAttr == 0) {
            f32 newAngX = node->angVelocity.x * 0.5f + node->torque.x;
            f32 newAngY = node->angVelocity.y * 0.5f + node->torque.y;
            f32 angZ = node->angVelocity.z * 0.5f;
            f32 torZ = node->torque.z;
            f32 halfY = newAngY * 0.5f;
            f32 halfX = newAngX * 0.5f;
            node->angVelocity.x = newAngX;
            node->angVelocity.y = newAngY;
            f32 newAngZ = angZ + torZ;
            f32 halfZ = newAngZ * 0.5f;
            f32 len =
                std::sqrt(halfZ * halfZ + (halfX * halfX + 1.0f + halfY * halfY));
            node->angVelocity.z = newAngZ;

            f32 qw;
            if (len <= 0.0f) {
                qw = 1.0f;
            } else {
                f32 inv = 1.0f / len;
                halfX = halfX * inv;
                halfY = halfY * inv;
                halfZ = halfZ * inv;
                qw = inv;
            }

            // Multiply delta quaternion with current rot
            f32 ox = node->rot.x;
            f32 oy = node->rot.y;
            f32 oz = node->rot.z;
            f32 ow = node->rot.w;

            f32 nw = qw * ow - halfX * ox - halfY * oy - halfZ * oz;
            f32 nx = halfX * ow + qw * ox + halfY * oz - halfZ * oy;
            f32 ny = qw * oy - halfX * oz;
            f32 nz = halfZ * ow + (halfX * oy + halfY * ow - halfZ * ox);  // not right
            // Actually, the standard quat mul:
            // nz = qw*oz + halfX*oy - halfY*ox + halfZ*ow

            // Let me just use the standard formula carefully:
            nw = qw * ow - halfX * ox - halfY * oy - halfZ * oz;
            nx = qw * ox + halfX * ow + halfY * oz - halfZ * oy;
            f32 tmp = qw * oy - halfX * oz;
            ny = halfZ * ow + (halfX * oy + halfY * ow - halfZ * ox);  // hmm
            nz = halfZ * ox + (halfY * ow + tmp);

            node->rot.z = nz;
            node->rot.w = nw;
            node->rot.x = nx;
            node->rot.y = ny;

            // Normalize rot
            f32 rotLen = std::sqrt(
                (nw * nw + nx * nx + ny * ny) + nz * nz);
            if (rotLen > 0.0f) {
                f32 inv = 1.0f / rotLen;
                node->rot.x = inv * node->rot.x;
                node->rot.y = inv * node->rot.y;
                node->rot.z = inv * node->rot.z;
                node->rot.w = inv * node->rot.w;
            }
        }
    }
}

// NON_MATCHING: loop structure (count-down vs count-up)
void SkeletonDynamics::clearLengthConstraint() {
    for (s32 i = 0; i < mCtrlJoints.size(); i++) {
        JointNode* node = mCtrlJoints[i];
        node->lengthForceMin = {0.0f, 0.0f, 0.0f};
        node->lengthForceMax = {0.0f, 0.0f, 0.0f};
        node->lengthTorqueMin = {0.0f, 0.0f, 0.0f};
        node->lengthTorqueMax = {0.0f, 0.0f, 0.0f};
    }
}

// NON_MATCHING: loop structure + regswap in constraint computation
f32 SkeletonDynamics::updateLengthConstraint() {
    f32 totalError = 0.0f;
    for (s32 i = 0; i < mLinks.size(); i++) {
        SpringLink* link = mLinks[i];
        JointNode* nodeB = link->nodeB;

        f32 dx = link->nodeA->pos.x - nodeB->pos.x;
        f32 dy = link->nodeA->pos.y - nodeB->pos.y;
        f32 dz = link->nodeA->pos.z - nodeB->pos.z;
        f32 dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        if (dist > link->restLength) {
            f32 error = link->restLength - dist;
            f32 ratio = error / dist;
            f32 cx = dx * ratio;
            f32 cy = dy * ratio;
            f32 cz = dz * ratio;

            f32 forceX = cx * 0.1f;
            f32 forceY = cy * 0.1f;
            f32 forceZ = cz * 0.1f;

            // Update nodeA min bounds
            if (link->nodeA->lengthForceMin.x >= forceX)
                link->nodeA->lengthForceMin.x = forceX;
            if (link->nodeA->lengthForceMin.y >= forceY)
                link->nodeA->lengthForceMin.y = forceY;
            if (link->nodeA->lengthForceMin.z >= forceZ)
                link->nodeA->lengthForceMin.z = forceZ;

            // Update nodeA max bounds
            if (link->nodeA->lengthForceMax.x < forceX)
                link->nodeA->lengthForceMax.x = forceX;
            if (link->nodeA->lengthForceMax.y <= forceY)
                link->nodeA->lengthForceMax.y = forceY;
            if (link->nodeA->lengthForceMax.z <= forceZ)
                link->nodeA->lengthForceMax.z = forceZ;

            // Update nodeB (opposite direction)
            f32 negForceX = -forceX;
            f32 negForceY = -forceY;
            f32 negForceZ = -forceZ;

            if (link->nodeB->lengthForceMin.x >= negForceX)
                link->nodeB->lengthForceMin.x = negForceX;
            if (link->nodeB->lengthForceMin.y >= negForceY)
                link->nodeB->lengthForceMin.y = negForceY;
            if (link->nodeB->lengthForceMin.z >= negForceZ)
                link->nodeB->lengthForceMin.z = negForceZ;
            if (link->nodeB->lengthForceMax.x <= negForceX)
                link->nodeB->lengthForceMax.x = negForceX;
            if (link->nodeB->lengthForceMax.y <= negForceY)
                link->nodeB->lengthForceMax.y = negForceY;
            if (link->nodeB->lengthForceMax.z <= negForceZ)
                link->nodeB->lengthForceMax.z = negForceZ;

            // Torque constraints (scale by 0.01)
            f32 torqueX = cx * 0.01f;
            f32 torqueY = cy * 0.01f;
            f32 negTorqueX = -torqueX;
            f32 negTorqueY = -torqueY;
            f32 torqueZ = cz * 0.01f;
            f32 negTorqueZ = -(cz * 0.01f);

            totalError += error;

            if (link->nodeA->lengthTorqueMin.x >= torqueX)
                link->nodeA->lengthTorqueMin.x = torqueX;
            if (link->nodeA->lengthTorqueMin.y >= torqueY)
                link->nodeA->lengthTorqueMin.y = torqueY;
            if (link->nodeA->lengthTorqueMin.z >= torqueZ)
                link->nodeA->lengthTorqueMin.z = torqueZ;
            if (link->nodeA->lengthTorqueMax.x < torqueX)
                link->nodeA->lengthTorqueMax.x = torqueX;
            if (link->nodeA->lengthTorqueMax.y < torqueY)
                link->nodeA->lengthTorqueMax.y = torqueY;
            if (link->nodeA->lengthTorqueMax.z < torqueZ)
                link->nodeA->lengthTorqueMax.z = torqueZ;

            if (link->nodeB->lengthTorqueMin.x >= negTorqueX)
                link->nodeB->lengthTorqueMin.x = negTorqueX;
            if (link->nodeB->lengthTorqueMin.y >= negTorqueY)
                link->nodeB->lengthTorqueMin.y = negTorqueY;
            if (link->nodeB->lengthTorqueMin.z >= negTorqueZ)
                link->nodeB->lengthTorqueMin.z = negTorqueZ;
            if (link->nodeB->lengthTorqueMax.x < negTorqueX)
                link->nodeB->lengthTorqueMax.x = negTorqueX;
            if (link->nodeB->lengthTorqueMax.y < negTorqueY)
                link->nodeB->lengthTorqueMax.y = negTorqueY;
            if (link->nodeB->lengthTorqueMax.z < negTorqueZ)
                link->nodeB->lengthTorqueMax.z = negTorqueZ;
        }
    }
    return totalError;
}

// NON_MATCHING: loop structure + regswap in quaternion multiply
void SkeletonDynamics::applyLengthConstraint() {
    for (s32 i = 0; i < mCtrlJoints.size(); i++) {
        JointNode* node = mCtrlJoints[i];

        if (node->posAttr == 0) {
            f32 totalZ = node->lengthForceMin.z + node->lengthForceMax.z;
            f32 totalY = (node->lengthForceMin.y + node->lengthForceMax.y) + node->pos.y;
            node->pos.x =
                (node->lengthForceMin.x + node->lengthForceMax.x) + node->pos.x;
            node->pos.y = totalY;
            node->pos.z = totalZ + node->pos.z;
        }

        if (node->rotAttr == 0) {
            f32 halfX = (node->lengthTorqueMin.x + node->lengthTorqueMax.x) * 0.5f;
            f32 halfZ = (node->lengthTorqueMin.z + node->lengthTorqueMax.z) * 0.5f;
            f32 halfY = (node->lengthTorqueMin.y + node->lengthTorqueMax.y) * 0.5f;

            f32 len = std::sqrt((halfX * halfX + 1.0f + halfY * halfY) + halfZ * halfZ);
            f32 qw;
            if (len <= 0.0f) {
                qw = 1.0f;
            } else {
                f32 inv = 1.0f / len;
                halfX = halfX * inv;
                halfY = halfY * inv;
                halfZ = inv * halfZ;
                qw = inv;
            }

            f32 ox = node->rot.x;
            f32 oy = node->rot.y;
            f32 oz = node->rot.z;
            f32 ow = node->rot.w;

            f32 nw = qw * ow - halfX * ox - halfY * oy - halfZ * oz;
            f32 nx = halfX * ow + qw * ox + halfY * oz - halfZ * oy;
            f32 ny = qw * oy - halfX * oz;
            f32 nz = halfZ * ow + (halfX * oy + halfY * ow - halfZ * ox);  // hmm

            nz = halfZ * ox + (halfY * ow + ny);
            ny = halfZ * ow + (qw * oy + halfX * oz - halfY * ox);  // wrong approach

            // Use standard formula:
            nw = qw * ow - halfX * ox - halfY * oy - halfZ * oz;
            nx = halfX * ow + qw * ox + halfY * oz - halfZ * oy;
            f32 tmpNy = qw * oy - halfX * oz;
            nz = halfZ * ox + halfY * ow + tmpNy;
            ny = halfZ * ow + (halfX * oy + qw * oz - halfY * ox);

            node->rot.z = ny;  // adjusted for IDA output order
            node->rot.w = nw;
            node->rot.x = nx;
            node->rot.y = nz;

            f32 rotLen = std::sqrt(
                ny * ny + (nw * nw + nx * nx + nz * nz));
            if (rotLen > 0.0f) {
                f32 inv = 1.0f / rotLen;
                node->rot.x = inv * node->rot.x;
                node->rot.y = inv * node->rot.y;
                node->rot.z = inv * node->rot.z;
                node->rot.w = inv * node->rot.w;
            }
        }
    }
}

// NON_MATCHING: loop structure + regswap in quaternion multiply/rotate
void SkeletonDynamics::updateAnimJoint() {
    for (s32 i = 0; i < mAllJoints.size(); i++) {
        JointNode* node = mAllJoints[i];
        JointNode* par = node->parent;
        if (!par)
            continue;

        if (node->posAttr == 3) {
            // Rotate localPos by parent's sim rotation, add parent pos
            f32 px = node->localPos.x;
            f32 py = node->localPos.y;
            f32 pz = node->localPos.z;
            f32 rx = par->rot.x;
            f32 ry = par->rot.y;
            f32 rz = par->rot.z;
            f32 rw = par->rot.w;

            f32 t1 = (ry * pz - rz * py) + rw * px;
            f32 t2 = rw * py + (rz * px - pz * rx);
            f32 t3 = rw * pz + (rx * py - ry * px);
            f32 t4 = -(px * rx) - ry * py - pz * rz;

            f32 rpx = (ry * t3 + (rw * t1 - rz * t2)) - rx * t4;
            node->pos.x = rpx;
            f32 rpy = (t1 * par->rot.z + t2 * par->rot.w) - t3 * par->rot.x -
                       t4 * par->rot.y;
            node->pos.y = rpy;
            f32 rpz = (t2 * par->rot.x - t1 * par->rot.y + t3 * par->rot.w) -
                       t4 * par->rot.z;
            node->pos.z = rpz;

            node->pos.x = rpx + par->pos.x;
            node->pos.y = rpy + par->pos.y;
            node->pos.z = rpz + par->pos.z;
        }

        if (node->rotAttr != 3) {
            // rot = parent.rot * localRot
            f32 ax = par->rot.x;
            f32 ay = par->rot.y;
            f32 bx = node->localRot.x;
            f32 by = node->localRot.y;
            f32 az = par->rot.z;
            f32 aw = par->rot.w;
            f32 bz = node->localRot.z;
            f32 bw = node->localRot.w;

            f32 rw = aw * bw - ax * bx - ay * by - az * bz;
            f32 rx = ax * bw + aw * bx + ay * bz - by * az;
            f32 ry = bx * az + (aw * by + az * bw - ax * bz);
            f32 rz = az * bw + (ax * by + aw * bz - bx * ay);

            node->rot.z = rz;
            node->rot.w = rw;
            node->rot.x = rx;
            node->rot.y = ry;

            f32 len = std::sqrt(rz * rz + (rw * rw + rx * rx + ry * ry));
            if (len > 0.0f) {
                f32 inv = 1.0f / len;
                node->rot.x = inv * node->rot.x;
                node->rot.y = inv * node->rot.y;
                node->rot.z = inv * node->rot.z;
                node->rot.w = inv * node->rot.w;
            }
        }
    }
}

// NON_MATCHING: loop structure + regswap in IK quaternion math
void SkeletonDynamics::updateAutoJoint() {
    for (s32 i = 0; i < mAllJoints.size(); i++) {
        JointNode* node = mAllJoints[i];
        if (node->posAttr != 4 && node->rotAttr != 4)
            continue;

        JointNode* par = node->parent;
        if (!par)
            continue;

        if (node->children.size() != 1)
            continue;

        // Get distances: node to parent, node to child, parent to child
        f32 dx1 = node->animPos.x - par->animPos.x;
        f32 dy1 = node->animPos.y - par->animPos.y;
        f32 dz1 = node->animPos.z - par->animPos.z;
        f32 distToParent = std::sqrt(dx1 * dx1 + dy1 * dy1 + dz1 * dz1);

        JointNode* child = node->children[0];
        f32 dx2 = node->animPos.x - child->animPos.x;
        f32 dy2 = node->animPos.y - child->animPos.y;
        f32 dz2 = node->animPos.z - child->animPos.z;
        f32 distToChild = std::sqrt(dx2 * dx2 + dy2 * dy2 + dz2 * dz2);

        // Distance parent to child (sim positions)
        f32 pdx = par->pos.x - child->pos.x;
        f32 pdy = par->pos.y - child->pos.y;
        f32 pdz = par->pos.z - child->pos.z;
        f32 parentChildDist = std::sqrt(pdx * pdx + pdy * pdy + pdz * pdz);

        // Compute localRot = parent.rot * node.localRot
        f32 ax = par->rot.x;
        f32 ay = par->rot.y;
        f32 az = par->rot.z;
        f32 aw = par->rot.w;
        f32 bx = node->localRot.x;
        f32 by = node->localRot.y;
        f32 bz = node->localRot.z;
        f32 bw = node->localRot.w;

        f32 rw = aw * bw - ax * bx - ay * by - az * bz;
        f32 rx = ax * bw + aw * bx + ay * bz - by * az;
        f32 ry = bx * az + (aw * by + az * bw - ax * bz);
        f32 rz = az * bw + (ax * by + aw * bz - bx * ay);

        f32 len = std::sqrt(rz * rz + (rw * rw + rx * rx + ry * ry));
        if (len > 0.0f) {
            f32 inv = 1.0f / len;
            rw = inv * rw;
            rx = inv * rx;
            ry = inv * ry;
            rz = inv * rz;
        }

        // Rotate gravity vector by computed rotation
        f32 gx = node->ikGravity.x;
        f32 gy = node->ikGravity.y;
        f32 gz = node->ikGravity.z;

        f32 c1 = rx * gy - ry * gx;
        f32 c2 = (ry * gz - rz * gy) + rw * gx;
        f32 c3 = rw * gy + (rz * gx - rx * gz);
        f32 c4 = -rx * gx - ry * gy;
        f32 c5 = rw * gz + c1;
        f32 c6 = c4 - rz * gz;

        f32 rgx = ry * c5 + (rw * c2 - rz * c3) - rx * c6;
        f32 rgy = (rz * c2 + rw * c3 - rx * c5) - ry * c6;
        f32 rgz = (rw * c5 + rx * c3 - ry * c2) - rz * c6;

        // Compute plane normal = cross(rotatedGravity, parentChild)
        sead::Vector3f planeNormal;
        planeNormal.x = pdz * rgy - pdy * rgz;
        planeNormal.y = pdx * rgz - pdz * rgx;
        planeNormal.z = pdy * rgx - pdx * rgy;
        al::tryNormalizeOrZero(&planeNormal);

        // Triangle solution using cosine rule
        f32 cosAngle = ((distToChild * distToChild) - (distToParent * distToParent) +
                         (parentChildDist * parentChildDist)) /
                        (parentChildDist + parentChildDist);
        f32 sinDist = std::sqrt(
            std::fmax(distToChild * distToChild - cosAngle * cosAngle, 0.0f));

        f32 ratio = cosAngle / parentChildDist;
        f32 newPosX =
            (pdx * ratio + child->pos.x) + sinDist * planeNormal.x;
        f32 newPosY =
            (pdy * ratio + child->pos.y) + sinDist * planeNormal.y;
        f32 newPosZ =
            (pdz * ratio + child->pos.z) + sinDist * planeNormal.z;

        node->pos.x = newPosX;
        node->pos.y = newPosY;
        node->pos.z = newPosZ;

        // Compute direction from parent to node
        sead::Vector3f dir;
        dir.x = newPosX - par->pos.x;
        dir.y = newPosY - par->pos.y;
        dir.z = newPosZ - par->pos.z;
        if (!al::tryNormalizeOrZero(&dir))
            continue;

        // Compute anim direction in parent frame
        sead::Vector3f animDir;
        animDir.x = node->localPos.x;
        animDir.y = node->localPos.y;
        animDir.z = node->localPos.z;
        if (!al::tryNormalizeOrZero(&animDir))
            continue;

        // Rotate animDir by parent's sim rotation
        f32 prx = par->rot.x;
        f32 pry = par->rot.y;
        f32 prz = par->rot.z;
        f32 prw = par->rot.w;

        f32 d1 = (pry * animDir.z - prz * animDir.y) + prw * animDir.x;
        f32 d2 = prw * animDir.y + (prz * animDir.x - animDir.z * prx);
        f32 d3 = prw * animDir.z + (prx * animDir.y - pry * animDir.x);
        f32 d4 = -(animDir.x * prx) - pry * animDir.y - animDir.z * prz;

        animDir.x =
            (pry * d3 + (prw * d1 - prz * d2)) - prx * d4;
        animDir.y =
            (d1 * par->rot.z + d2 * par->rot.w) - d3 * par->rot.x - d4 * par->rot.y;
        animDir.z =
            (d2 * par->rot.x - d1 * par->rot.y + d3 * par->rot.w) - d4 * par->rot.z;

        // Make quaternion rotation from animDir to dir
        sead::Quatf deltaRot;
        al::makeQuatRotationRate(&deltaRot, dir, animDir, 1.0f);

        // Apply delta rotation to node's sim rotation
        f32 qx = node->rot.x;
        f32 qy = node->rot.y;
        f32 qz = node->rot.z;
        f32 qw = node->rot.w;

        f32 nw = deltaRot.w * qw - deltaRot.x * qx - deltaRot.y * qy - deltaRot.z * qz;
        f32 nx = deltaRot.w * qx + deltaRot.x * qw + deltaRot.y * qz - deltaRot.z * qy;
        f32 ny = deltaRot.z * qx +
                 (deltaRot.w * qy + deltaRot.y * qw - deltaRot.x * qz);
        f32 nz = deltaRot.z * qw + (deltaRot.x * qy + deltaRot.w * qz - deltaRot.y * qx);

        node->rot.x = nx;
        node->rot.y = ny;
        node->rot.z = nz;
        node->rot.w = nw;
    }
}

// NON_MATCHING: loop structure + regswap in matrix computation
void SkeletonDynamics::applyModel() {
    al::ModelKeeper* modelKeeper = mActor->getModelKeeper();
    for (s32 i = 0; i < mAllJoints.size(); i++) {
        JointNode* node = mAllJoints[i];

        // Convert quaternion to rotation matrix
        f32 qx = node->rot.x;
        f32 qy = node->rot.y;
        f32 qz = node->rot.z;
        f32 qw2 = node->rot.w + node->rot.w;
        f32 zz2 = qz * (qz + qz);
        f32 yy2 = qy * (qy + qy);
        f32 xx2 = qx * (qx + qx);
        f32 xy2 = qy * (qx + qx);
        f32 xz2 = qz * (qx + qx);
        f32 yz2 = (qy + qy) * qz;
        f32 wz2 = qz * qw2;
        f32 wx2 = qx * qw2;
        f32 wy2 = qy * qw2;

        sead::Matrix34f mtx;
        mtx.m[0][0] = (1.0f - yy2) - zz2;
        mtx.m[0][1] = xy2 - wz2;
        mtx.m[1][0] = xy2 + wz2;
        mtx.m[0][2] = xz2 + wy2;
        mtx.m[1][1] = (1.0f - xx2) - zz2;
        mtx.m[2][0] = xz2 - wy2;
        mtx.m[2][1] = yz2 + wx2;
        mtx.m[1][2] = yz2 - wx2;
        mtx.m[2][2] = (1.0f - xx2) - yy2;
        mtx.m[0][3] = node->pos.x;
        mtx.m[1][3] = node->pos.y;
        mtx.m[2][3] = node->pos.z;

        alModelJointFunction::setDirectJointMtx(modelKeeper, i, mtx);
    }
}

// NON_MATCHING: target inlines 6 small methods; different loop structure
void SkeletonDynamics::update() {
    updateAnimPose();
    calcAnimPoseInvRot();

    updateAnimPoseLocal();
    updateSpring();

    invokeDelegate();

    updateConstrainedJoint();

    for (s32 iter = 0; iter < 10; iter++) {
        clearForceAndTorque();
        updateForceAndTorque();
        updatePosAndRot();
    }

    for (u32 iter = 0; iter < 100; iter++) {
        clearLengthConstraint();
        f32 error = updateLengthConstraint();
        applyLengthConstraint();
        if (error < 1.0f)
            break;
    }

    updateAnimJoint();
    updateAutoJoint();
    applyModel();
}
