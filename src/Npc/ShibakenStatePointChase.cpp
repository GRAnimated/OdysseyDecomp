#include "Npc/ShibakenStatePointChase.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "MapObj/DigPoint.h"
#include "Npc/ActorStateReactionBase.h"
#include "Npc/Shibaken.h"
#include "Npc/ShibakenFunction.h"
#include "Npc/ShibakenMoveAnimCtrl.h"
#include "Npc/ShibakenStateJump.h"
#include "Npc/ShibakenStateTurn.h"
#include "Npc/ShibakenStateWait.h"
#include "Util/PlayerUtil.h"

namespace {
NERVE_IMPL(ShibakenStatePointChase, FirstWait);
NERVE_IMPL(ShibakenStatePointChase, FindTurn);
NERVE_IMPL(ShibakenStatePointChase, PointTurn);
NERVE_IMPL(ShibakenStatePointChase, Jump);
NERVE_IMPL(ShibakenStatePointChase, Reaction);
NERVE_IMPL(ShibakenStatePointChase, ChaseRun);
NERVE_IMPL(ShibakenStatePointChase, Find);
NERVE_IMPL(ShibakenStatePointChase, ChaseWalk);
NERVE_IMPL(ShibakenStatePointChase, ChaseWalkSniffNear);
NERVE_IMPL(ShibakenStatePointChase, ChaseWalkSniff);
NERVE_IMPL(ShibakenStatePointChase, ChaseFind);
NERVE_IMPL(ShibakenStatePointChase, Sniff);

NERVES_MAKE_NOSTRUCT(ShibakenStatePointChase, FirstWait, FindTurn, PointTurn, Jump, Reaction,
                     ChaseRun, Find, ChaseWalk, ChaseWalkSniffNear, ChaseWalkSniff, ChaseFind,
                     Sniff);

void calcRandomChaseOffset(sead::Vector3f* outOffset, al::LiveActor* actor, f32 distance) {
    sead::Vector3f front = al::getFront(actor);
    const sead::Vector3f& gravity = al::getGravity(actor);
    al::rotateVectorDegree(&front, front, gravity, al::getRandomDegree());

    if (al::isNearZero(*outOffset, 0.001f)) {
        outOffset->x = front.x * distance;
        outOffset->y = front.y * distance;
        outOffset->z = front.z * distance;
        return;
    }

    f32 dot = outOffset->x * front.x + outOffset->y * front.y + outOffset->z * front.z;
    if (dot > 0.0f) {
        front.x = -front.x;
        front.y = -front.y;
        front.z = -front.z;
    }

    outOffset->x = front.x * distance;
    outOffset->y = front.y * distance;
    outOffset->z = front.z * distance;
}
}  // namespace

ShibakenStatePointChase::ShibakenStatePointChase(const char* name, Shibaken* shibaken,
                                                 ShibakenMoveAnimCtrl* moveAnimCtrl,
                                                 ActorStateReactionBase* reaction)
    : HostStateBase<Shibaken>(name, shibaken), mMoveAnimCtrl(moveAnimCtrl), mReaction(reaction) {
    initNerve(&FirstWait, 5);
    mStateWait = new ShibakenStateWait(u8"初回待機", shibaken, true);
    mStateTurn = new ShibakenStateTurn(u8"ターン", shibaken);
    mStateJump = new ShibakenStateJump(u8"ジャンプ", shibaken);
    al::initNerveState(this, mStateWait, &FirstWait, u8"初回待機");
    al::initNerveState(this, mStateTurn, &FindTurn, u8"プレイヤーへターン");
    al::initNerveState(this, mStateTurn, &PointTurn, u8"ポイントへターン");
    al::initNerveState(this, mStateJump, &Jump, u8"ジャンプ");
    al::initNerveState(this, mReaction, &Reaction, u8"リアクション");
}

void ShibakenStatePointChase::appear() {
    al::NerveStateBase::appear();
    _7c = 0;
}

void ShibakenStatePointChase::kill() {
    al::NerveStateBase::kill();
    mCurrentPoint = nullptr;
    mMoveAnimCtrl->endWalkSniff();
    mMoveAnimCtrl->mIsSniffing = false;
}

void ShibakenStatePointChase::startFirstWait(DigPoint* point) {
    mCurrentPoint = point;
    al::setNerve(this, &FirstWait);
}

void ShibakenStatePointChase::startChaseRun(DigPoint* point) {
    mCurrentPoint = point;
    al::setNerve(this, &ChaseRun);
}

bool ShibakenStatePointChase::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                                         al::HitSensor* self) {
    return ShibakenFunction::tryStartReaction(this, mReaction, &Reaction, msg, other, self);
}

bool ShibakenStatePointChase::isKillByDeathArea() const {
    return mStateJump->isKillByDeathArea();
}

void ShibakenStatePointChase::exeFirstWait() {
    al::updateNerveState(this);
    if (!mStateWait->isPlayingWait())
        return;
    if (!rs::isNearPlayerH(getHost(), 750.0f))
        return;
    ShibakenStateTurn* turn = mStateTurn;
    const sead::Vector3f& playerPos = rs::getPlayerPos(getHost());
    if (turn->tryStartTurn(playerPos))
        al::setNerve(this, &FindTurn);
    else
        al::setNerve(this, &Find);
}

void ShibakenStatePointChase::exeFindTurn() {
    al::updateNerveStateAndNextNerve(this, &Find);
}

// NON_MATCHING: vector math uses individual loads instead of ldp pairs
void ShibakenStatePointChase::exeFind() {
    if (al::isFirstStep(this))
        al::startAction(getHost(), "Find");
    if (!al::isActionEnd(getHost()))
        return;

    calcRandomChaseOffset(&_70, mCurrentPoint, 500.0f);
    ShibakenStateTurn* turn = mStateTurn;
    const sead::Vector3f& trans = al::getTrans(mCurrentPoint);
    sead::Vector3f target;
    target.x = trans.x + _70.x;
    target.y = trans.y + _70.y;
    target.z = trans.z + _70.z;
    if (turn->tryStartTurn(target))
        al::setNerve(this, &PointTurn);
    else
        al::setNerve(this, &ChaseRun);
}

void ShibakenStatePointChase::exePointTurn() {
    al::updateNerveStateAndNextNerve(this, &ChaseRun);
}

// NON_MATCHING: vector math uses individual loads instead of ldp pairs; host not paired with
// mCurrentPoint
void ShibakenStatePointChase::exeChaseRun() {
    if (al::isFirstStep(this))
        al::startAction(getHost(), "Move");

    Shibaken* host = getHost();
    const sead::Vector3f& trans = al::getTrans(mCurrentPoint);
    sead::Vector3f target;
    target.x = trans.x + _70.x;
    target.y = trans.y + _70.y;
    target.z = trans.z + _70.z;
    ShibakenFunction::chaseToTargetRun(host, target);

    if (ShibakenFunction::tryStartJump(this, &Jump))
        return;

    host = getHost();
    const sead::Vector3f& trans2 = al::getTrans(mCurrentPoint);
    target.x = trans2.x + _70.x;
    target.y = trans2.y + _70.y;
    target.z = trans2.z + _70.z;
    if (al::isNear(host, target, 600.0f)) {
        mMoveAnimCtrl->mIsSniffing = true;
        al::setNerve(this, &ChaseWalk);
    }
}

// NON_MATCHING: vector math uses individual loads instead of ldp pairs
void ShibakenStatePointChase::exeChaseWalk() {
    if (al::isFirstStep(this))
        al::tryStartActionIfNotPlaying(getHost(), "Move");

    Shibaken* host = getHost();
    const sead::Vector3f& trans = al::getTrans(mCurrentPoint);
    sead::Vector3f target;
    target.x = trans.x + _70.x;
    target.y = trans.y + _70.y;
    target.z = trans.z + _70.z;
    ShibakenFunction::chaseToTargetWalk(host, target);

    if (ShibakenFunction::tryStartJump(this, &Jump))
        return;

    if (al::isNear(getHost(), al::getTrans(mCurrentPoint), 160.0f))
        al::setNerve(this, &ChaseWalkSniffNear);
    else if (al::isGreaterEqualStep(this, 120))
        al::setNerve(this, &ChaseWalkSniff);
}

// NON_MATCHING: vector math uses individual loads instead of ldp pairs
void ShibakenStatePointChase::exeChaseWalkSniff() {
    if (al::isFirstStep(this))
        mMoveAnimCtrl->startWalkSniff();

    Shibaken* host = getHost();
    const sead::Vector3f& trans = al::getTrans(mCurrentPoint);
    sead::Vector3f target;
    target.x = trans.x + _70.x;
    target.y = trans.y + _70.y;
    target.z = trans.z + _70.z;
    ShibakenFunction::chaseToTargetWalkSniff(host, target);

    if (ShibakenFunction::tryStartJump(this, &Jump))
        return;
    if (!al::isGreaterEqualStep(this, 120))
        return;

    mMoveAnimCtrl->endWalkSniff();

    if (al::isNerve(this, &ChaseWalkSniffNear) || al::isNear(getHost(), mCurrentPoint, 160.0f)) {
        al::setNerve(this, &ChaseFind);
        return;
    }

    f32 dist = al::calcDistance(getHost(), mCurrentPoint);
    if (dist >= 500.0f) {
        const sead::Vector3f& hostTrans = al::getTrans(getHost());
        const sead::Vector3f& pointTrans = al::getTrans(mCurrentPoint);
        f32 dx = hostTrans.x - pointTrans.x;
        f32 dy = hostTrans.y - pointTrans.y;
        f32 dz = hostTrans.z - pointTrans.z;
        f32 rawDist = sead::Mathf::sqrt(dx * dx + dy * dy + dz * dz);
        f32 norm = al::normalize(rawDist, 0.0f, 600.0f);
        calcRandomChaseOffset(&_70, mCurrentPoint, norm * 500.0f);
    } else {
        _70.x = 0.0f;
        _70.y = 0.0f;
        _70.z = 0.0f;
        _7c = 0;
    }
    al::setNerve(this, &ChaseWalk);
}

void ShibakenStatePointChase::exeChaseWalkSniffNear() {
    exeChaseWalkSniff();
}

void ShibakenStatePointChase::exeChaseFind() {
    if (al::isFirstStep(this))
        mMoveAnimCtrl->mIsSniffing = false;
    const sead::Vector3f& trans = al::getTrans(mCurrentPoint);
    if (ShibakenFunction::executeFindTurnNerve(this, trans, &_50, &_60))
        al::setNerve(this, &Sniff);
}

void ShibakenStatePointChase::exeSniff() {
    if (al::isFirstStep(this))
        al::startAction(getHost(), "Sniff");

    _7c++;

    if (!mCurrentPoint->_108) {
        kill();
        return;
    }

    if (_7c == 15) {
        if (al::isDead(mCurrentPoint))
            mCurrentPoint->appear();
        if (!mCurrentPoint->_110)
            mCurrentPoint->emitGlowEffect();
    }

    if (_7c >= 15) {
        if (al::isDead(mCurrentPoint)) {
            kill();
            return;
        }
        if (_7c >= 300) {
            kill();
            return;
        }
    }
    ShibakenFunction::addFallVelocityToGround(getHost(), -1.0f);
}

// NON_MATCHING: afterSubState body should be inlined within exeJump's range (IPO)
void ShibakenStatePointChase::exeJump() {
    if (!al::updateNerveState(this))
        return;
    if (mStateJump->isKillByDeathArea()) {
        kill();
        return;
    }
    afterSubState(mCurrentPoint, &_70);
}

void ShibakenStatePointChase::exeReaction() {
    if (ShibakenFunction::executeReactionNerve(this))
        afterSubState(mCurrentPoint, &_70);
}

void ShibakenStatePointChase::afterSubState(DigPoint* point, sead::Vector3f* offset) {
    if (!point->_108) {
        kill();
        return;
    }

    if (al::isAlive(point) && point->_110) {
        sead::Vector3f downDir = sead::Vector3f::zero;
        al::calcDownDir(&downDir, getHost());
        al::scaleVelocityExceptDirection(getHost(), downDir, 0.0f);
        al::setNerve(this, &Sniff);
        return;
    }

    sead::Vector3f target = al::getTrans(point);
    target.x += offset->x;
    target.y += offset->y;
    target.z += offset->z;

    if (!al::isNear(getHost(), target, 600.0f)) {
        al::setNerve(this, &ChaseRun);
        return;
    }

    if (al::isNear(getHost(), point, 160.0f)) {
        sead::Vector3f downDir = sead::Vector3f::zero;
        al::calcDownDir(&downDir, getHost());
        al::scaleVelocityExceptDirection(getHost(), downDir, 0.0f);
        al::setNerve(this, &ChaseFind);
        return;
    }

    al::setNerve(this, &ChaseWalkSniff);
}
