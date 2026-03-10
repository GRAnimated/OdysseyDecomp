#include "Npc/JumpingRopeLayout.h"

#include "Library/Layout/LayoutActorUtil.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Layout/MiniGameCueLayout.h"
#include "Layout/MiniGameLayout.h"
#include "MapObj/RouteGuideDirector.h"
#include "Npc/JumpingRopeNpc.h"
#include "Util/InputInterruptTutorialUtil.h"

namespace {
NERVE_IMPL(JumpingRopeLayout, Wait);
NERVE_IMPL(JumpingRopeLayout, Game);
NERVE_IMPL(JumpingRopeLayout, Result);
NERVE_IMPL(JumpingRopeLayout, End);
NERVE_IMPL(JumpingRopeLayout, DisplayTutorial);
NERVES_MAKE_NOSTRUCT(JumpingRopeLayout, Wait, Game, Result, End, DisplayTutorial);
}  // namespace

JumpingRopeLayout::JumpingRopeLayout(JumpingRopeNpc* npc, const al::ActorInitInfo& info)
    : al::NerveExecutor(u8"縄跳びレイアウト管理") {
    mNpc = npc;
    mMiniGameLayout = new MiniGameLayout(u8"ミニゲームレイアウト", al::getLayoutInitInfo(info));
    mCueLayout = new MiniGameCueLayout(u8"ミニゲーム見出しレイアウト", al::getLayoutInitInfo(info));
    initNerve(&Wait, 0);
}

void JumpingRopeLayout::exeWait() {
    if (mNpc->isNerveJump()) {
        if (mNpc->mJumpCount > 1u)
            al::setNerve(this, &Game);
        else if (al::isGreaterEqualStep(mNpc, 120))
            al::setNerve(this, &Game);
    }
}

void JumpingRopeLayout::exeGame() {
    if (al::isFirstStep(this)) {
        mMiniGameLayout->startJumprope();
        mMiniGameLayout->setBestCount(mNpc->getBest());
        rs::offRouteGuideByActor(mNpc);
    }
    mMiniGameLayout->setCount(mNpc->mJumpCount);
    if (!mNpc->isNerveJump()) {
        if (mNpc->isNerveMiss())
            al::setNerve(this, &Result);
        else {
            mMiniGameLayout->end();
            rs::onRouteGuideByActor(mNpc);
            al::setNerve(this, &Wait);
        }
    }
}

void JumpingRopeLayout::exeResult() {
    if (al::isFirstStep(this))
        mCueLayout->appearMiss();
    if (al::isStep(this, 180)) {
        mMiniGameLayout->end();
        rs::onRouteGuideByActor(mNpc);
        al::setNerve(this, &End);
    }
}

void JumpingRopeLayout::exeEnd() {
    if (al::isDead(mMiniGameLayout) && al::isDead(mCueLayout)) {
        if (mNpc->isPlayerOff())
            al::setNerve(this, &Wait);
        else
            al::setNerve(this, &DisplayTutorial);
    }
}

void JumpingRopeLayout::exeDisplayTutorial() {
    if (al::isFirstStep(this))
        rs::tryAppearObjectTutorial(mNpc, "JumpingRopeNpc");
    if (mNpc->isPlayerOff()) {
        if (rs::isShowObjectTutorial(mNpc, "JumpingRopeNpc"))
            rs::tryCloseObjectTutorial(mNpc, "JumpingRopeNpc");
        al::setNerve(this, &Wait);
    }
}

bool JumpingRopeLayout::isWaiting() const {
    return al::isNerve(this, &Wait);
}

void JumpingRopeLayout::setBest(u32 best) {
    mMiniGameLayout->setBestCount(best);
    mMiniGameLayout->startNewRecord();
}

JumpingRopeLayout::~JumpingRopeLayout() = default;
