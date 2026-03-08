#include "Npc/JumpingRopeNpc.h"

#include <cmath>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>

#include "Library/Base/StringUtil.h"
#include "Library/Bgm/BgmLineFunction.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Player/PlayerUtil.h"
#include "Library/Stage/StageSwitchUtil.h"

#include "Item/Shine.h"
#include "Npc/JumpingRopeLayout.h"
#include "System/GameDataUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/SensorMsgFunction.h"

struct ActionAnimInfo {
    al::LiveActor* actor;
    const char* actionName;
    f32 frame = 0.0f;
    f32 frameRate = 1.0f;
    f32 maxFrame;
    bool isLoop;
    const char* reactionAction = nullptr;
};

struct RopeNode {
    bool isActive;
    sead::Vector3f pos;
    sead::Vector3f targetPos;
    sead::Vector3f velocity;
};

namespace {

NERVE_IMPL(JumpingRopeNpc, Start);
NERVE_IMPL(JumpingRopeNpc, Wait);
NERVE_END_IMPL(JumpingRopeNpc, Jump);
NERVE_IMPL(JumpingRopeNpc, TryAgain);
NERVE_IMPL(JumpingRopeNpc, Miss);
NERVE_IMPL(JumpingRopeNpc, Interrupt);

NERVES_MAKE_STRUCT(JumpingRopeNpc, Start, Wait, Jump, TryAgain, Miss, Interrupt);

void updateActionAnimInfo(ActionAnimInfo* info) {
    if (info->reactionAction) {
        if (al::isActionEnd(info->actor)) {
            al::startAction(info->actor, info->actionName);
            info->reactionAction = nullptr;
            al::setActionFrame(info->actor, info->frame);
            al::setActionFrameRate(info->actor, info->frameRate);
        }
    } else {
        info->frame = al::getActionFrame(info->actor);
    }
    f32 newFrame = info->frame + info->frameRate;
    info->frame = newFrame;
    if (info->isLoop) {
        if (newFrame >= info->maxFrame)
            info->frame = al::modf(newFrame + info->maxFrame, info->maxFrame);
    } else {
        if (newFrame >= info->maxFrame)
            info->frame = info->maxFrame;
    }
}

void setActionAnimInfo(ActionAnimInfo* info, const char* actionName) {
    if (!info->reactionAction)
        al::startAction(info->actor, actionName);
    info->actionName = actionName;
    info->frame = 0.0f;
    info->frameRate = 1.0f;
    info->maxFrame = al::getActionFrameMax(info->actor, actionName);
    info->isLoop = !al::isActionOneTime(info->actor, actionName);
}

}  // namespace

JumpingRopeNpc::JumpingRopeNpc(const char* name) : al::LiveActor(name) {}

// NON_MATCHING: extremely complex init (2872 bytes)
void JumpingRopeNpc::init(const al::ActorInitInfo& info) {}

void JumpingRopeNpc::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isSensorNpc(self)) {
        al::sendMsgPush(other, self);
        rs::sendMsgPushToPlayer(other, self);
        return;
    }
    if (al::isSensorEye(self)) {
        f32 dist = al::calcDistance(other, self);
        f32 radius = al::getSensorRadius(self);
        if (dist < radius) {
            bool isSensorPlayer = al::isSensorPlayer(other);
            bool isEnemy = al::isSensorEnemyBody(other);
            if (!isEnemy)
                isEnemy = al::isSensorRide(other);
            if (!isSensorPlayer && !isEnemy)
                return;
            if (mCollisionEntries.ptrNum < mCollisionEntries.ptrNumMax) {
                void* entry = mCollisionFreeHead;
                if (entry)
                    mCollisionFreeHead = *(void**)entry;
                *(u64*)entry = 0;
                *((u64*)entry + 1) = 0;
                if (mCollisionEntries.ptrNum < mCollisionEntries.ptrNumMax) {
                    ((void**)mCollisionEntries.ptrs)[mCollisionEntries.ptrNum] = entry;
                    mCollisionEntries.ptrNum++;
                }
                const sead::Vector3f& sensorPos = al::getSensorPos(other);
                ((sead::Vector3f*)entry)->x = sensorPos.x;
                ((sead::Vector3f*)entry)->y = sensorPos.y;
                ((sead::Vector3f*)entry)->z = sensorPos.z;
                ((f32*)entry)[3] = al::getSensorRadius(other);
                if (isSensorPlayer)
                    *((u8*)&_180) = 1;
                if (isEnemy)
                    *((u8*)&_180 + 1) = 1;
            }
        }
    }
}

bool JumpingRopeNpc::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                                al::HitSensor* self) {
    if (al::isSensorNpc(self)) {
        if (mInvincibilityTimer == 0 && rs::isMsgCapReflect(message)) {
            ActionAnimInfo* info = mActionAnimInfo;
            mInvincibilityTimer = 6;
            if (al::isActionOneTime(info->actor, "Reaction")) {
                al::startAction(info->actor, "Reaction");
                info->reactionAction = "Reaction";
            }
            if (al::isNerve(this, &NrvJumpingRopeNpc.Jump) ||
                al::isNerve(this, &NrvJumpingRopeNpc.Start) ||
                al::isNerve(this, &NrvJumpingRopeNpc.Wait))
                miss();
            return true;
        }
        if (rs::isMsgPlayerObjLeapFrog(message) ||
            al::isMsgPlayerTrampleReflect(message) ||
            rs::isMsgPlayerAndCapObjHipDropReflectAll(message) ||
            rs::isMsgThrowObjHitReflect(message)) {
            ActionAnimInfo* info = mActionAnimInfo;
            if (al::isActionOneTime(info->actor, "Reaction")) {
                al::startAction(info->actor, "Reaction");
                info->reactionAction = "Reaction";
            }
            if (al::isNerve(this, &NrvJumpingRopeNpc.Jump) ||
                al::isNerve(this, &NrvJumpingRopeNpc.Start) ||
                al::isNerve(this, &NrvJumpingRopeNpc.Wait))
                miss();
            return true;
        }
    }
    if (rs::isMsgPlayerDisregardHomingAttack(message))
        return true;
    if (al::isMsgPlayerDisregard(message))
        return !al::isSensorName(self, "BodyUpper");
    return false;
}

bool JumpingRopeNpc::tryMiss() {
    if (al::isNerve(this, &NrvJumpingRopeNpc.Jump) ||
        al::isNerve(this, &NrvJumpingRopeNpc.Start) ||
        al::isNerve(this, &NrvJumpingRopeNpc.Wait)) {
        miss();
        return true;
    }
    return false;
}

void JumpingRopeNpc::movement() {
    void* freeHead = mCollisionFreeHead;
    for (s32 i = 0; i < mCollisionEntries.ptrNum; i++) {
        void* entry = ((void**)mCollisionEntries.ptrs)[i];
        *(void**)entry = freeHead;
        freeHead = entry;
        mCollisionFreeHead = entry;
    }
    mCollisionEntries.ptrNum = 0;
    _180 = 0;
    al::LiveActor::movement();
}

void JumpingRopeNpc::control() {
    _20d = false;
    if (mInvincibilityTimer != 0)
        mInvincibilityTimer--;
    mLayout->updateNerve();
}

void JumpingRopeNpc::updateAnim(bool sync) {
    updateActionAnimInfo(mActionAnimInfo);
    updateActionAnimInfo(mPartnerActionAnimInfo);
}

bool JumpingRopeNpc::isActionEnd() const {
    ActionAnimInfo* info = mActionAnimInfo;
    if (!info->reactionAction)
        return al::isActionEnd(info->actor);
    if (info->isLoop)
        return false;
    return info->frame == info->maxFrame;
}

void JumpingRopeNpc::restoreInterval() {
    mIntervalMul = mIntervalMul + (1.0f - mIntervalMul) * 0.01f;
}

bool JumpingRopeNpc::isEnableDisplayBalloon() const {
    const sead::Vector3f* playerPos = &al::getPlayerPos(this, 0);
    f32 dx = playerPos->x - mRopeCenter.x;
    f32 dy = playerPos->y - mRopeCenter.y;
    f32 dz = playerPos->z - mRopeCenter.z;
    return sqrtf(dx * dx + dy * dy + dz * dz) < 1200.0f;
}

void JumpingRopeNpc::interrupt() {
    mRopeSpeed = 0.0f;
    al::setNerve(this, &NrvJumpingRopeNpc.Interrupt);
}

void JumpingRopeNpc::startReactionPartner() {
    ActionAnimInfo* info = mPartnerActionAnimInfo;
    if (al::isActionOneTime(info->actor, "Reaction")) {
        al::startAction(info->actor, "Reaction");
        info->reactionAction = "Reaction";
    }
}

s32 JumpingRopeNpc::getBest() const {
    return rs::getJumpingRopeBestCount(this);
}

void JumpingRopeNpc::setBest(s32 count) {
    rs::setJumpingRopeBestCount(this, count);
}

void JumpingRopeNpc::setBestToday(s32 count) {
    rs::setJumpingRopeDayCount(this, count);
}

s32 JumpingRopeNpc::getBestToday() const {
    return rs::getJumpingRopeDayCount(this);
}

bool JumpingRopeNpc::isNerveJump() const {
    return al::isNerve(this, &NrvJumpingRopeNpc.Jump);
}

bool JumpingRopeNpc::isNerveMiss() const {
    return al::isNerve(this, &NrvJumpingRopeNpc.Miss);
}

void JumpingRopeNpc::miss() {
    if (!mLayout->isWaiting())
        al::startHitReaction(this, u8"失敗");
    mRopeSpeed = 0.0f;
    al::setNerve(this, &NrvJumpingRopeNpc.Miss);
    s32 best = getBest();
    if (best >= mJumpCount) {
        s32 today = getBestToday();
        if (today < mJumpCount)
            setBestToday(mJumpCount);
    } else {
        setBest(mJumpCount);
        setBestToday(mJumpCount);
        mLayout->setBest(mJumpCount);
    }
    rs::setJumpingRopeUpdateScoreFlag(this);
}

void JumpingRopeNpc::endJump() {
    rs::resetEventBalloonFilter(this);
    al::endBgmSituation(this, "JumpingRope", false);
}

void JumpingRopeNpc::startResultAction() {
    const char* actionName;
    if (mJumpCount < _200) {
        actionName = "ResultA";
    } else if (mJumpCount < _204) {
        actionName = "ResultB";
    } else {
        actionName = "ResultC";
    }
    setActionAnimInfo(mActionAnimInfo, actionName);
    setActionAnimInfo(mPartnerActionAnimInfo, actionName);
}

void JumpingRopeNpc::startResultMessage() {
    const char* message;
    if (mJumpCount < _200) {
        message = "ResultBad";
    } else if (mJumpCount >= _204) {
        message = "ResultCool";
    } else {
        message = "ResultNormal";
    }
    rs::startEventFlow(mEventFlowExecutor, message);
}

bool JumpingRopeNpc::tryStartResultLoopAction() {
    ActionAnimInfo* info = mActionAnimInfo;
    const char* waitAction;

    if (al::isEqualString("ResultA", info->actionName)) {
        waitAction = "WaitResultA";
    } else if (al::isEqualString("ResultB", info->actionName)) {
        waitAction = "WaitResultB";
    } else if (al::isEqualString("ResultC", info->actionName)) {
        waitAction = "WaitResultC";
    } else {
        return false;
    }

    if (!info->reactionAction) {
        if (!al::isActionEnd(info->actor))
            return false;
    } else {
        if (info->isLoop || info->frame != info->maxFrame)
            return false;
    }

    setActionAnimInfo(info, waitAction);
    setActionAnimInfo(mPartnerActionAnimInfo, waitAction);
    return true;
}

bool JumpingRopeNpc::isPlayerOff() const {
    const sead::Vector3f* playerPos = &al::getPlayerPos(this, 0);
    f32 dx = playerPos->x - mRopeCenter.x;
    f32 dy = playerPos->y - mRopeCenter.y;
    f32 dz = playerPos->z - mRopeCenter.z;
    return sqrtf(dx * dx + dy * dy + dz * dz) > mHalfRopeLength + mHalfRopeLength;
}

void JumpingRopeNpc::calcRopePos(sead::Vector3f* out, s32 idx, f32 t) const {
    sead::Vector3f p0, p1, p2, p3;
    s32 maxIdx = mRopeNodes.ptrNum - 1;
    s32 i0 = idx - 1;
    if (i0 < 0)
        i0 = 0;
    s32 i1 = idx;
    s32 i2 = idx + 1;
    if (i2 > maxIdx)
        i2 = maxIdx;
    s32 i3 = idx + 2;
    if (i3 > maxIdx)
        i3 = maxIdx;
    p0 = ((RopeNode*)((void**)mRopeNodes.ptrs)[i0])->pos;
    p1 = ((RopeNode*)((void**)mRopeNodes.ptrs)[i1])->pos;
    p2 = ((RopeNode*)((void**)mRopeNodes.ptrs)[i2])->pos;
    p3 = ((RopeNode*)((void**)mRopeNodes.ptrs)[i3])->pos;
    al::hermiteVec(out, p0, p1, p2, p3, t);
}

// NON_MATCHING: complex search logic
s32 JumpingRopeNpc::searchNearestNode() {
    sead::Vector3f frontDir;
    al::calcFrontDir(&frontDir, this);

    RopeNode* firstNode =
        mRopeNodes.ptrNum > 0 ? (RopeNode*)((void**)mRopeNodes.ptrs)[0] : nullptr;
    const sead::Vector3f* playerPos = &al::getPlayerPos(this, 0);

    s32 nodeCount = mRopeNodes.ptrNum - 2;
    if (nodeCount < 4)
        return 2;

    f32 baseDot = (playerPos->x - firstNode->pos.x) * frontDir.x +
                  (playerPos->y - firstNode->pos.y) * frontDir.y +
                  (playerPos->z - firstNode->pos.z) * frontDir.z;

    RopeNode* node2 =
        mRopeNodes.ptrNum >= 3 ? (RopeNode*)((void**)mRopeNodes.ptrs)[2] : nullptr;
    f32 dot2 = (node2->pos.x - firstNode->pos.x) * frontDir.x +
               (node2->pos.y - firstNode->pos.y) * frontDir.y +
               (node2->pos.z - firstNode->pos.z) * frontDir.z;
    f32 minDiff = dot2 - baseDot;
    if (minDiff <= 0.0f)
        minDiff = -minDiff;

    s32 result = 2;
    for (s32 i = 3; i < nodeCount; i++) {
        RopeNode* node = (RopeNode*)((void**)mRopeNodes.ptrs)[i];
        f32 dot = (node->pos.x - firstNode->pos.x) * frontDir.x +
                  (node->pos.y - firstNode->pos.y) * frontDir.y +
                  (node->pos.z - firstNode->pos.z) * frontDir.z;
        f32 diff = dot - baseDot;
        if (diff <= 0.0f)
            diff = -diff;
        if (diff < minDiff) {
            minDiff = diff;
            result = i;
        }
    }
    return result;
}

// NON_MATCHING: complex rotation update with sin/cos
void JumpingRopeNpc::updateRot(bool updateFrame) {
    sead::Vector3f sideDir;
    sead::Vector3f frontDir;
    al::calcSideDir(&sideDir, this);
    al::calcFrontDir(&frontDir, this);

    RopeNode* firstNode =
        mRopeNodes.ptrNum > 0 ? (RopeNode*)((void**)mRopeNodes.ptrs)[0] : nullptr;
    f32 cosVal = cosf(mAngle);
    f32 sinVal = sinf(mAngle);

    RopeNode* secondNode =
        mRopeNodes.ptrNum >= 2 ? (RopeNode*)((void**)mRopeNodes.ptrs)[1] : nullptr;
    secondNode->pos.x = sinVal * sideDir.x * mNodeInterval + firstNode->pos.x +
                         frontDir.x * 65.0f;
    secondNode->pos.y = sinVal * sideDir.y * mNodeInterval + cosVal * mNodeInterval +
                         firstNode->pos.y + frontDir.y * 65.0f;
    secondNode->pos.z = sinVal * sideDir.z * mNodeInterval + firstNode->pos.z +
                         frontDir.z * 65.0f;

    s32 count = mRopeNodes.ptrNum;
    RopeNode* lastNode = (RopeNode*)((void**)mRopeNodes.ptrs)[count - 1];
    RopeNode* secondLastNode =
        count >= 2 ? (RopeNode*)((void**)mRopeNodes.ptrs)[count - 2] : nullptr;
    secondLastNode->pos.x = sinVal * sideDir.x * mNodeInterval + lastNode->pos.x -
                             frontDir.x * 65.0f;
    secondLastNode->pos.y = sinVal * sideDir.y * mNodeInterval + cosVal * mNodeInterval +
                             lastNode->pos.y - frontDir.y * 65.0f;
    secondLastNode->pos.z = sinVal * sideDir.z * mNodeInterval + lastNode->pos.z -
                             frontDir.z * 65.0f;

    if (updateFrame) {
        f32 phase =
            al::modf((mAngle + sead::Mathf::pi()) / sead::Mathf::pi2() + 1.0f, 1.0f);
        f32 maxFrame = al::getActionFrameMax(this, "TurnLeft");
        f32 animFrame = phase * maxFrame;
        ActionAnimInfo* animInfo = mActionAnimInfo;
        if (!animInfo->reactionAction)
            al::setActionFrame(animInfo->actor, animFrame);
        animInfo->frame = animFrame;
        ActionAnimInfo* partnerAnimInfo = mPartnerActionAnimInfo;
        if (!partnerAnimInfo->reactionAction)
            al::setActionFrame(partnerAnimInfo->actor, animFrame);
        partnerAnimInfo->frame = animFrame;
    }

    f32 speedRad = sead::Mathf::deg2rad(mRopeSpeed);
    f32 curAngle = mAngle;
    if (curAngle > 4.7124f)
        speedRad = speedRad * 1.1f;
    f32 newAngle = al::modf(curAngle + speedRad + sead::Mathf::pi2(), sead::Mathf::pi2());

    if (newAngle >= mAngle) {
        _20e = false;
    } else {
        _20e = _1fc;
        _20c = false;
        _1fc = true;
        if (!al::isNerve(this, &NrvJumpingRopeNpc.Miss) &&
            !al::isNerve(this, &NrvJumpingRopeNpc.Interrupt) &&
            !al::isNerve(this, &NrvJumpingRopeNpc.TryAgain) &&
            !al::isNerve(this, &NrvJumpingRopeNpc.Start))
            al::startHitReaction(this, u8"風切音");
    }

    mAngle = newAngle;
    if (mRopeSpeed > 0.0f)
        _20d = true;
}

// NON_MATCHING: massive rope physics simulation (1756 bytes)
void JumpingRopeNpc::updateRope() {
    if (al::isNerve(this, &NrvJumpingRopeNpc.Miss) ||
        al::isNerve(this, &NrvJumpingRopeNpc.Interrupt) ||
        al::isNerve(this, &NrvJumpingRopeNpc.TryAgain)) {
        RopeNode* secondNode =
            mRopeNodes.ptrNum >= 2
                ? (RopeNode*)((void**)mRopeNodes.ptrs)[1]
                : nullptr;
        al::calcJointPos(&secondNode->pos, this, "RopeRoot");

        s32 count = mRopeNodes.ptrNum;
        RopeNode* secondLastNode =
            count >= 2
                ? (RopeNode*)((void**)mRopeNodes.ptrs)[count - 2]
                : nullptr;
        al::calcJointPos(&secondLastNode->pos, mPartner, "RopeRoot");
    }

    s32 nodeCount = mRopeNodes.ptrNum;
    for (s32 i = 0; i < nodeCount; i++) {
        RopeNode* node = (RopeNode*)((void**)mRopeNodes.ptrs)[i];
        node->targetPos = node->pos;
        if (!node->isActive) {
            node->velocity.y += -0.3f;
            node->pos.x += node->velocity.x;
            node->pos.y += node->velocity.y;
            node->pos.z += node->velocity.z;
        }
    }

    for (s32 iter = 0; iter < 10; iter++) {
        for (s32 i = 0; i < nodeCount - 1; i++) {
            RopeNode* nodeA = (RopeNode*)((void**)mRopeNodes.ptrs)[i];
            RopeNode* nodeB = (RopeNode*)((void**)mRopeNodes.ptrs)[i + 1];
            sead::Vector3f diff;
            diff.x = nodeB->pos.x - nodeA->pos.x;
            diff.y = nodeB->pos.y - nodeA->pos.y;
            diff.z = nodeB->pos.z - nodeA->pos.z;
            f32 dist = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
            if (dist < 0.1f) {
                diff.x = nodeB->targetPos.x - nodeA->targetPos.x;
                diff.y = nodeB->targetPos.y - nodeA->targetPos.y;
                diff.z = nodeB->targetPos.z - nodeA->targetPos.z;
                dist = 0.0f;
            }
            al::normalize(&diff);
            f32 excess = dist - mNodeInterval * mIntervalMul;
            f32 cx = diff.x * excess;
            f32 cy = diff.y * excess;
            f32 cz = diff.z * excess;
            if (nodeA->isActive) {
                if (!nodeB->isActive) {
                    nodeB->pos.x -= cx;
                    nodeB->pos.y -= cy;
                    nodeB->pos.z -= cz;
                }
            } else if (nodeB->isActive) {
                nodeA->pos.x += cx;
                nodeA->pos.y += cy;
                nodeA->pos.z += cz;
            } else {
                nodeA->pos.x += cx * 0.5f;
                nodeA->pos.y += cy * 0.5f;
                nodeA->pos.z += cz * 0.5f;
                nodeB->pos.x -= cx * 0.5f;
                nodeB->pos.y -= cy * 0.5f;
                nodeB->pos.z -= cz * 0.5f;
            }
        }
    }

    const sead::Vector3f& trans = al::getTrans(this);
    for (s32 i = 0; i < mRopeNodes.ptrNum; i++) {
        RopeNode* node = (RopeNode*)((void**)mRopeNodes.ptrs)[i];
        if (node->pos.y - 10.0f < trans.y) {
            node->pos.y = trans.y + 10.0f;
            if (!(_180 & 0xFF)) {
                if (_180 >= 0x100)
                    al::startHitReaction(this,
                                         u8"地面パチン");
                _20c = true;
            }
        }
        if (!node->isActive) {
            node->velocity.x = node->pos.x - node->targetPos.x;
            node->velocity.y = node->pos.y - node->targetPos.y;
            node->velocity.z = node->pos.z - node->targetPos.z;
        }
    }

    if (mRopeNodes.ptrNum >= 2) {
        RopeNode* sn = (RopeNode*)((void**)mRopeNodes.ptrs)[1];
        mHandTarget = sn->pos;
        RopeNode* sln =
            (RopeNode*)((void**)mRopeNodes.ptrs)[mRopeNodes.ptrNum - 2];
        mPartnerHandTarget = sln->pos;
    }
}

// NON_MATCHING: complex collision detection (680 bytes)
bool JumpingRopeNpc::checkRopeCollision() const {
    return false;
}

// NON_MATCHING: extremely complex NEON quaternion math (4804 bytes)
void JumpingRopeNpc::calcAndSetJointMtx(sead::Matrix34f* mtx, s32 jointIdx) {}

// NON_MATCHING: complex start with alpha fade and TurnLeftStart/TurnRightStart
void JumpingRopeNpc::exeStart() {
    if (al::isFirstStep(this)) {
        setActionAnimInfo(mActionAnimInfo, "TurnLeftStart");
        setActionAnimInfo(mPartnerActionAnimInfo, "TurnRightStart");
        mAngle = sead::Mathf::pi();
        mRopeSpeed = 0.0f;
        al::validateClipping(this);
        mJumpCount = 0;
    }

    updateActionAnimInfo(mActionAnimInfo);
    updateActionAnimInfo(mPartnerActionAnimInfo);

    if (al::isStep(this, 85))
        mRopeSpeed = 5.0f;

    updateRot(false);
    updateRope();

    if (checkRopeCollision()) {
        miss();
        return;
    }

    if (!isActionEnd())
        return;

    setActionAnimInfo(mActionAnimInfo, "TurnLeft");
    setActionAnimInfo(mPartnerActionAnimInfo, "TurnRight");
    al::setNerve(this, &NrvJumpingRopeNpc.Wait);
}

// NON_MATCHING: event flow and distance checks
void JumpingRopeNpc::exeWait() {
    if (al::isFirstStep(this)) {
        rs::startEventFlow(mEventFlowExecutor, "Cmon");
        al::validateClipping(this);
    }

    updateActionAnimInfo(mActionAnimInfo);
    updateActionAnimInfo(mPartnerActionAnimInfo);

    updateRot(true);
    updateRope();
    restoreInterval();

    const sead::Vector3f* playerPos = &al::getPlayerPos(this, 0);
    f32 dx = playerPos->x - mRopeCenter.x;
    f32 dy = playerPos->y - mRopeCenter.y;
    f32 dz = playerPos->z - mRopeCenter.z;
    if (sqrtf(dx * dx + dy * dy + dz * dz) < 1200.0f)
        rs::updateEventFlow(mEventFlowExecutor);

    if (checkRopeCollision()) {
        mRopeSpeed = 0.0f;
        al::setNerve(this, &NrvJumpingRopeNpc.Interrupt);
        return;
    }

    playerPos = &al::getPlayerPos(this, 0);
    dx = playerPos->x - mRopeCenter.x;
    dy = playerPos->y - mRopeCenter.y;
    dz = playerPos->z - mRopeCenter.z;
    if (sqrtf(dx * dx + dy * dy + dz * dz) >= mHalfRopeLength * 1.5f)
        return;

    mJumpCount = 0;
    al::setNerve(this, &NrvJumpingRopeNpc.Jump);
}

// NON_MATCHING: extremely complex jump logic (1340 bytes)
void JumpingRopeNpc::exeJump() {
    if (al::isFirstStep(this)) {
        rs::startEventFlow(mEventFlowExecutor, "Ready");
        rs::startEventFlow(mPartnerEventFlowExecutor, "SpeedUp");
        _1f8 = 0;
        al::invalidateClipping(this);
        rs::setEventBalloonFilterOnlyMiniGame(this);
        al::startBgmSituation(this, "JumpingRope", false);
        _20e = false;
    }

    updateActionAnimInfo(mActionAnimInfo);
    updateActionAnimInfo(mPartnerActionAnimInfo);
    updateRot(true);
    updateRope();
    restoreInterval();

    if (mCollisionEntries.ptrNum == 0)
        rs::updateEventFlow(mEventFlowExecutor);

    if (_20e && mJumpCount >= 1) {
        miss();
        return;
    }

    const sead::Vector3f* playerPos = &al::getPlayerPos(this, 0);
    f32 dx = playerPos->x - mSensorPos.x;
    f32 dy = playerPos->y - mSensorPos.y;
    f32 dz = playerPos->z - mSensorPos.z;
    f32 distToSensor = sqrtf(dx * dx + dy * dy + dz * dz);
    f32 sensorRadius = al::getSensorRadius(this, "RopeDomain");

    if (_1fc) {
        if (distToSensor < sensorRadius) {
            s32 nearestIdx = searchNearestNode();
            RopeNode* nearNode =
                (RopeNode*)((void**)mRopeNodes.ptrs)[nearestIdx];
            if (nearNode->pos.y < al::getPlayerPos(this, 0).y) {
                sead::Vector3f front;
                al::calcFrontDir(&front, this);
                sead::Vector3f cross;
                cross.x = front.y * 0.0f - front.z * 1.0f;
                cross.y = front.z * 0.0f - 0.0f * front.x;
                cross.z = 1.0f * front.x - front.y * 0.0f;

                playerPos = &al::getPlayerPos(this, 0);
                f32 dot1 = cross.x * (nearNode->pos.x - playerPos->x) +
                           cross.y * (nearNode->pos.y - playerPos->y) +
                           cross.z * (nearNode->pos.z - playerPos->z);

                RopeNode* nextNode =
                    (RopeNode*)((void**)mRopeNodes.ptrs)[nearestIdx];
                playerPos = &al::getPlayerPos(this, 0);
                f32 dot2 =
                    cross.x * (nextNode->targetPos.x - playerPos->x) +
                    cross.y * (nextNode->targetPos.y - playerPos->y) +
                    cross.z * (nextNode->targetPos.z - playerPos->z);

                if (!al::isSameSign(dot1, dot2)) {
                    s32 count = mJumpCount + 1;
                    if (count >= 99999)
                        count = 99999;
                    mJumpCount = count;

                    if (count == _204)
                        al::startHitReaction(this,
                                             u8"１００回記念");
                    else if (count > _204 && count % _204 == 0)
                        al::startHitReaction(this,
                                             u8"１００毎記念");
                    else
                        al::startHitReaction(this,
                                             u8"カウントアップ");

                    if (mRopeSpeed < 10.0f && mJumpCount % 5 == 0) {
                        mRopeSpeed = fminf(mRopeSpeed + 0.5f, 10.0f);
                        _1f8 = al::getNerveStep(this) + 60;
                        al::startHitReaction(this,
                                             u8"スピードアップ");
                    }

                    _1fc = false;
                    if (mJumpCount == _200) {
                        Shine* shine1 = mShine1;
                        if (shine1 && !_1d8) {
                            shine1->appearPopup();
                            _1d8 = true;
                        }
                        al::tryOnStageSwitch(this, "ClearSwitchOn");
                        if (mShine2 && mShine2->isGot())
                            rs::requestSwitchTalkNpcEventJumpingRope(this, 2);
                        else
                            rs::requestSwitchTalkNpcEventJumpingRope(this, 1);
                    }
                    if (mJumpCount == _204) {
                        Shine* shine2 = mShine2;
                        if (shine2 && !_1d9) {
                            shine2->appearPopup();
                            _1d9 = true;
                        }
                        al::tryOnStageSwitch(this, "ClearSwitch2On");
                        rs::requestSwitchTalkNpcEventJumpingRope(this, 2);
                    }
                }
            }
        }
    }

    if (al::isLessStep(this, _1f8))
        rs::updateEventFlow(mPartnerEventFlowExecutor);

    if (checkRopeCollision()) {
        if (distToSensor >= sensorRadius && mJumpCount < 1) {
            mRopeSpeed = 0.0f;
            al::setNerve(this, &NrvJumpingRopeNpc.Interrupt);
            return;
        }
        miss();
        return;
    }

    playerPos = &al::getPlayerPos(this, 0);
    dx = playerPos->x - mRopeCenter.x;
    dy = playerPos->y - mRopeCenter.y;
    dz = playerPos->z - mRopeCenter.z;
    if (sqrtf(dx * dx + dy * dy + dz * dz) < mHalfRopeLength * 1.5f)
        return;

    if (mJumpCount < 1 || mLayout->isWaiting()) {
        al::setNerve(this, &NrvJumpingRopeNpc.Wait);
        return;
    }

    miss();
}

// NON_MATCHING: model alpha writes and complex result/event flow
void JumpingRopeNpc::exeMiss() {
    if (al::isFirstStep(this)) {
        startResultAction();
        startResultMessage();
        _182 = false;
    }

    const sead::Vector3f* playerPos = &al::getPlayerPos(this, 0);
    f32 dx = playerPos->x - mRopeCenter.x;
    f32 dy = playerPos->y - mRopeCenter.y;
    f32 dz = playerPos->z - mRopeCenter.z;
    if (sqrtf(dx * dx + dy * dy + dz * dz) < 1200.0f)
        rs::updateEventFlow(mEventFlowExecutor);

    updateActionAnimInfo(mActionAnimInfo);
    updateActionAnimInfo(mPartnerActionAnimInfo);

    mAngle = mAngle + (sead::Mathf::pi() - mAngle) * 0.1f;

    updateRot(false);
    updateRope();
    restoreInterval();

    ActionAnimInfo* info = mActionAnimInfo;
    if (!info->isLoop) {
        if (info->reactionAction) {
            if (info->frame == info->maxFrame)
                tryStartResultLoopAction();
        } else {
            if (al::isActionEnd(info->actor))
                tryStartResultLoopAction();
        }
    }

    if (al::isGreaterEqualStep(this, 180) && mActionAnimInfo->isLoop) {
        if (_182 && !(*((u8*)&_180 + 1))) {
            al::setNerve(this, &NrvJumpingRopeNpc.TryAgain);
            return;
        }
        playerPos = &al::getPlayerPos(this, 0);
        dx = playerPos->x - mRopeCenter.x;
        dy = playerPos->y - mRopeCenter.y;
        dz = playerPos->z - mRopeCenter.z;
        if (sqrtf(dx * dx + dy * dy + dz * dz) > mHalfRopeLength + mHalfRopeLength &&
            mCollisionEntries.ptrNum == 0 && mLayout->isWaiting()) {
            mIntervalMul = 0.95f;
            al::setNerve(this, &NrvJumpingRopeNpc.Start);
        }
    }
}

// NON_MATCHING: model alpha writes and complex result action logic
void JumpingRopeNpc::exeInterrupt() {
    if (al::isFirstStep(this)) {
        setActionAnimInfo(mActionAnimInfo, "ResultA");
        setActionAnimInfo(mPartnerActionAnimInfo, "ResultA");
        rs::startEventFlow(mEventFlowExecutor, "Obstacle");
        _182 = true;
    }

    const sead::Vector3f* playerPos = &al::getPlayerPos(this, 0);
    f32 dx = playerPos->x - mRopeCenter.x;
    f32 dy = playerPos->y - mRopeCenter.y;
    f32 dz = playerPos->z - mRopeCenter.z;
    if (sqrtf(dx * dx + dy * dy + dz * dz) < 1200.0f)
        rs::updateEventFlow(mEventFlowExecutor);

    updateActionAnimInfo(mActionAnimInfo);
    updateActionAnimInfo(mPartnerActionAnimInfo);

    mAngle = mAngle + (sead::Mathf::pi() - mAngle) * 0.1f;

    updateRot(false);
    updateRope();
    restoreInterval();

    ActionAnimInfo* info = mActionAnimInfo;
    if (info->reactionAction) {
        if (info->isLoop || info->frame != info->maxFrame)
            return;
    } else {
        if (!al::isActionEnd(info->actor))
            return;
    }

    tryStartResultLoopAction();

    playerPos = &al::getPlayerPos(this, 0);
    dx = playerPos->x - mRopeCenter.x;
    dy = playerPos->y - mRopeCenter.y;
    dz = playerPos->z - mRopeCenter.z;
    if (sqrtf(dx * dx + dy * dy + dz * dz) > mHalfRopeLength + mHalfRopeLength &&
        mCollisionEntries.ptrNum == 0 && mLayout->isWaiting()) {
        mIntervalMul = 0.95f;
        al::setNerve(this, &NrvJumpingRopeNpc.Start);
    } else {
        al::setNerve(this, &NrvJumpingRopeNpc.TryAgain);
    }
}

// NON_MATCHING: event flow obstacle/result switching
void JumpingRopeNpc::exeTryAgain() {
    updateActionAnimInfo(mActionAnimInfo);
    updateActionAnimInfo(mPartnerActionAnimInfo);

    mAngle = mAngle + (sead::Mathf::pi() - mAngle) * 0.1f;

    updateRot(false);
    updateRope();
    restoreInterval();

    const sead::Vector3f* playerPos = &al::getPlayerPos(this, 0);
    f32 dx = playerPos->x - mRopeCenter.x;
    f32 dy = playerPos->y - mRopeCenter.y;
    f32 dz = playerPos->z - mRopeCenter.z;
    if (sqrtf(dx * dx + dy * dy + dz * dz) > mHalfRopeLength + mHalfRopeLength &&
        mCollisionEntries.ptrNum == 0 && mLayout->isWaiting()) {
        mIntervalMul = 0.95f;
        al::setNerve(this, &NrvJumpingRopeNpc.Start);
        return;
    }

    if (_182) {
        if (!(*((u8*)&_180 + 1))) {
            rs::startEventFlow(mEventFlowExecutor, "Obstacle");
            _182 = true;
        }
    } else {
        if (*((u8*)&_180 + 1)) {
            rs::startEventFlow(mEventFlowExecutor, "ResultBad");
            _182 = false;
        }
    }

    playerPos = &al::getPlayerPos(this, 0);
    dx = playerPos->x - mRopeCenter.x;
    dy = playerPos->y - mRopeCenter.y;
    dz = playerPos->z - mRopeCenter.z;
    if (sqrtf(dx * dx + dy * dy + dz * dz) < 1200.0f)
        rs::updateEventFlow(mEventFlowExecutor);
}

