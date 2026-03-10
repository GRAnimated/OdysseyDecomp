#include "Npc/HomeCap.h"

#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/HomeCapFunction.h"
#include "Npc/HomeCapStateMoveChair.h"
#include "Npc/HomeCapStateMoveSouvenir.h"
#include "Util/CapManHeroDemoUtil.h"

namespace {
NERVE_IMPL(HomeCap, Hide);
NERVE_IMPL(HomeCap, MoveOtherChair);
NERVE_IMPL(HomeCap, MoveSouvenir);
NERVE_IMPL(HomeCap, WaitOtherChair);
NERVE_IMPL(HomeCap, WanderSign);
NERVE_IMPL(HomeCap, NoMoveSouvenirSleepWait);
NERVE_IMPL(HomeCap, SleepWait);
NERVE_IMPL(HomeCap, SleepStart);
NERVE_IMPL(HomeCap, Sleep);
NERVE_IMPL(HomeCap, SleepEnd);
NERVES_MAKE_NOSTRUCT(HomeCap, Hide, MoveOtherChair, MoveSouvenir, WaitOtherChair, WanderSign,
                     NoMoveSouvenirSleepWait, SleepWait, SleepStart, Sleep, SleepEnd);

sead::Vector3f sAimChairOffset(0.0f, 100.0f, 30.0f);
}  // namespace

HomeCap::HomeCap(const char* name, HomeChair* chair,
                 const sead::PtrArray<HomeCapMovePoint>& movePoints)
    : al::LiveActor(name) {
    mChair = chair;
    mMoveChairState = nullptr;
    mMoveSouvenirState = nullptr;
    mMovePoints = &movePoints;
}

void HomeCap::init(const al::ActorInitInfo& initInfo) {
    al::initActorWithArchiveName(this, initInfo, "CapManHero", "Home");
    al::initNerve(this, &Hide, 2);

    mMoveChairState = new HomeCapStateMoveChair(this);
    al::initNerveState(this, mMoveChairState, &MoveOtherChair, u8"椅子に移動");

    mMoveSouvenirState = new HomeCapStateMoveSouvenir(this, *mMovePoints);
    al::initNerveState(this, mMoveSouvenirState, &MoveSouvenir, u8"お土産移動");

    al::initJointControllerKeeper(this, 1);
    CapManHeroDemoUtil::initCapManHeroTailJointController(this);
    makeActorDead();
}

void HomeCap::appearMoveOtherChair(al::LiveActor* otherChair) {
    al::LiveActor::appear();
    CapManHeroDemoUtil::startCapManHeroCommonSettingAfterShowModel(this);
    mMoveChairState->appearMoveOtherChair(otherChair);
    mMoveSouvenirState->setReturnChair(otherChair);
    al::setNerve(this, &MoveOtherChair);
}

// NON_MATCHING: target tail-calls kill() via BR; our compiler uses BLR + explicit return true
bool HomeCap::tryKillReturnHead() {
    if (!al::isAlive(this))
        return false;
    al::startHitReaction(this, u8"消滅");
    kill();
    return true;
}

void HomeCap::exeHide() {}

void HomeCap::exeMoveOtherChair() {
    al::updateNerveStateAndNextNerve(this, &WaitOtherChair);
}

void HomeCap::exeWaitOtherChair() {
    if (al::isFirstStep(this))
        al::startAction(this, "HomeWait");

    if (al::isGreaterEqualStep(this, 710)) {
        if (mMoveSouvenirState->tryMoveSouvenir())
            al::setNerve(this, &WanderSign);
        else
            al::setNerve(this, &NoMoveSouvenirSleepWait);
    }
}

void HomeCap::exeWanderSign() {
    if (al::isFirstStep(this))
        al::startAction(this, "SouvenirMoveSign");

    if (al::isActionEnd(this))
        al::setNerve(this, &MoveSouvenir);
}

void HomeCap::exeMoveSouvenir() {
    if (al::updateNerveState(this))
        al::setNerve(this, &SleepWait);
}

void HomeCap::exeSleepWait() {
    if (al::isFirstStep(this))
        al::startAction(this, "HomeWait");

    if (al::isGreaterEqualStep(this, 600))
        al::setNerve(this, &SleepStart);
}

void HomeCap::exeNoMoveSouvenirSleepWait() {
    if (al::isFirstStep(this))
        al::startAction(this, "HomeWait");

    if (al::isGreaterEqualStep(this, 2400))
        al::setNerve(this, &SleepStart);
}

void HomeCap::exeSleepStart() {
    if (al::isFirstStep(this))
        al::startAction(this, "HomeSleepStart");

    if (al::isActionEnd(this))
        al::setNerve(this, &Sleep);
}

void HomeCap::exeSleep() {
    if (al::isFirstStep(this))
        al::startAction(this, "HomeSleep");

    if (al::isGreaterEqualStep(this, 600))
        al::setNerve(this, &SleepEnd);
}

void HomeCap::exeSleepEnd() {
    if (al::isFirstStep(this))
        al::startAction(this, "HomeSleepEnd");

    if (al::isActionEnd(this))
        al::setNerve(this, &WaitOtherChair);
}

const sead::Vector3f& HomeCapFunction::getAimChairOffset() {
    return sAimChairOffset;
}
