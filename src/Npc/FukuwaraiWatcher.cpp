#include "Npc/FukuwaraiWatcher.h"

#include "Library/Base/StringUtil.h"
#include "Library/LiveActor/ActorAreaFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/MapObj/FixMapParts.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Se/SeFunction.h"
#include "Library/Bgm/BgmLineFunction.h"
#include "Library/Stage/StageSwitchUtil.h"

#include "Npc/FukuwaraiFaceParts.h"
#include "Npc/FukuwaraiNpc.h"
#include "Util/DemoUtil.h"

namespace {
NERVE_IMPL(FukuwaraiWatcher, Wait);
NERVE_IMPL(FukuwaraiWatcher, SetStartPosition);
NERVE_IMPL(FukuwaraiWatcher, Memorize);
NERVE_IMPL(FukuwaraiWatcher, Play);
NERVE_IMPL(FukuwaraiWatcher, WaitStartResultEnd);
NERVE_IMPL(FukuwaraiWatcher, ResultWait);
NERVE_IMPL(FukuwaraiWatcher, ResultAppearParts);
NERVE_IMPL(FukuwaraiWatcher, End);
NERVES_MAKE_NOSTRUCT(FukuwaraiWatcher, Wait, SetStartPosition, Memorize, Play,
                     WaitStartResultEnd, ResultWait, ResultAppearParts, End);
}  // namespace

FukuwaraiWatcher::FukuwaraiWatcher(const char* name) : al::LiveActor(name) {}

void FukuwaraiWatcher::init(const al::ActorInitInfo& info) {
    al::initActorSceneInfo(this, info);
    al::initExecutorMapObjMovement(this, info);
    al::initStageSwitch(this, info);
    al::initActorBgmKeeper(this, info, nullptr);
    al::initActorSeKeeperWithout3D(this, info, "FukuwaraiWatcher");

    al::AreaObjGroup* areaGroup = al::createLinkAreaGroup(
        this, info, "FukuwaraiArea", "子供エリアグループ", "子供エリア");

    s32 partsCount = al::calcLinkChildNum(info, "FaceParts");

    auto* group = new al::DeriveActorGroup<FukuwaraiFaceParts>("顔パーツ", partsCount);
    mPartsGroup = group;

    for (s32 i = 0; i < partsCount; i++) {
        auto* parts = new FukuwaraiFaceParts("顔パーツ", areaGroup);
        al::initLinksActor(parts, info, "FaceParts", i);
        mPartsGroup->registerActor(parts);
        parts->kill();
    }

    auto* face = new al::FixMapParts("福笑い顔完成図");
    mFace = face;
    al::initLinksActor(face, info, "FukuwaraiFace", 0);
    mFace->appear();

    auto* faceLine = new al::FixMapParts("福笑い顔輪郭下絵");
    mFaceLine = faceLine;
    al::initLinksActor(faceLine, info, "FukuwaraiFaceLine", 0);
    mFaceLine->kill();

    auto* faceSilhouette = new al::FixMapParts("福笑い顔シルエット");
    mFaceSilhouette = faceSilhouette;
    al::initLinksActor(faceSilhouette, info, "FukuwaraiFaceSilhouette", 0);
    mFaceLine->kill();

    mIsMarioFace = al::isEqualString("FukuwaraiMarioFace", al::getModelName(mFace));

    auto* npc = new FukuwaraiNpc("福笑いNPC", this, mFace, mIsMarioFace);
    mNpc = npc;
    al::initLinksActor(npc, info, "FukuwaraiNpc", 0);

    al::initNerve(this, &Wait, 0);
    makeActorAlive();
}

void FukuwaraiWatcher::control() {}

s32 FukuwaraiWatcher::getPartsNum() const {
    return mPartsGroup->getActorCount();
}

FukuwaraiFaceParts* FukuwaraiWatcher::getParts(s32 index) const {
    return mPartsGroup->getDeriveActor(index);
}

void FukuwaraiWatcher::exeWait() {
    if (!mNpc->isSetStartPosition())
        return;

    al::onStageSwitch(this, "SwitchFukuwaraiOn");
    al::onStageSwitch(this, "SwitchExplainOn");

    if (al::isDead(mFace))
        mFace->appear();
    mFaceLine->kill();

    s32 count = mPartsGroup->getActorCount();
    for (s32 i = 0; i < count; i++)
        mPartsGroup->getDeriveActor(i)->kill();

    al::setNerve(this, &SetStartPosition);
}

void FukuwaraiWatcher::exeSetStartPosition() {
    if (al::isFirstStep(this))
        al::startSe(this, "SetStartPos");

    if (mNpc->isMemorize())
        al::setNerve(this, &Memorize);
}

void FukuwaraiWatcher::exeMemorize() {
    if (mNpc->isPlay()) {
        al::offStageSwitch(this, "SwitchExplainOn");
        al::setNerve(this, &Play);
    }
}

void FukuwaraiWatcher::exePlay() {
    if (al::isFirstStep(this)) {
        mFaceLine->appear();
        al::setModelAlphaMask(mFaceLine, 1.0f);
        mFace->kill();

        for (s32 i = 0; i < mPartsGroup->getActorCount(); i++)
            mPartsGroup->getDeriveActor(i)->reset();

        al::startSe(this, "PlayStart");
        al::startBgm(this, "MiniGame", -1, 60);
    }

    if (al::isGreaterStep(this, 120) && mNpc->getNextLv() == 1) {
        auto* faceLine = mFaceLine;
        f32 rate = al::calcNerveRate(this, 120, 360);
        al::setModelAlphaMask(faceLine, 1.0f - rate);
    }

    if (mNpc->isWaitStartResult()) {
        mNpc->startResultWipe();
        al::setNerve(this, &WaitStartResultEnd);
    }
}

// NON_MATCHING: score accumulator uses one FP reg instead of two; loop comparison 64-bit vs 32-bit
void FukuwaraiWatcher::exeWaitStartResultEnd() {
    if (al::isFirstStep(this)) {
        al::disableBgmLineChange(this);
        al::stopBgm(this, "MiniGame", 30);
    }

    if (!mNpc->isEventStartResultEnd())
        return;

    al::onStageSwitch(this, "SwitchResultOn");

    f32 totalScore = 0.0f;
    s32 partsNum = mPartsGroup->getActorCount();
    if (partsNum >= 1) {
        f32 scoreSum = mPartsGroup->getDeriveActor(0)->calcScore(mIsMarioFace);
        for (s32 i = 1; i < partsNum; i++)
            scoreSum += mPartsGroup->getDeriveActor(i)->calcScore(mIsMarioFace);

        if (scoreSum + 0.5f >= 0.0f) {
            totalScore = scoreSum + 0.5f;
            if (totalScore > 100.0f)
                totalScore = 100.0f;
        }
    } else {
        totalScore = 0.5f;
    }

    mNpc->startResult((s32)totalScore);

    s32 count = mPartsGroup->getActorCount();
    for (s32 i = 0; i < count; i++) {
        auto* parts = mPartsGroup->getDeriveActor(i);
        rs::addDemoActor(parts, false);
        if (!parts->isPlaced())
            parts->vanish();
    }

    al::setNerve(this, &ResultWait);
}

// NON_MATCHING: score accumulator uses one FP reg instead of two; loop comparison 64-bit vs 32-bit
s32 FukuwaraiWatcher::calcTotalScore() const {
    f32 totalScore = 0.0f;
    s32 partsNum = mPartsGroup->getActorCount();
    if (partsNum >= 1) {
        f32 scoreSum = mPartsGroup->getDeriveActor(0)->calcScore(mIsMarioFace);
        for (s32 i = 1; i < partsNum; i++)
            scoreSum += mPartsGroup->getDeriveActor(i)->calcScore(mIsMarioFace);

        if (scoreSum + 0.5f >= 0.0f) {
            totalScore = scoreSum + 0.5f;
            if (totalScore > 100.0f)
                totalScore = 100.0f;
        }
    } else {
        totalScore = 0.5f;
    }

    return (s32)totalScore;
}

void FukuwaraiWatcher::exeResultWait() {
    if (al::isFirstStep(this)) {
        mFaceLine->appear();
        al::setModelAlphaMask(mFaceLine, 1.0f);

        s32 count = mPartsGroup->getActorCount();
        for (s32 i = 0; i < count; i++) {
            auto* parts = mPartsGroup->getDeriveActor(i);
            if (parts->isPlaced())
                mSortedParts.pushBack(parts);
        }

        mSortedParts.sort();
    }

    if (al::isGreaterStep(this, 60))
        al::setNerve(this, &ResultAppearParts);
}

// NON_MATCHING: erase(0) not inlined; target manually memmoves and decrements count
void FukuwaraiWatcher::exeResultAppearParts() {
    if (al::isIntervalStep(this, 15, 0) && mSortedParts.size() >= 1) {
        mSortedParts.front()->show();
        mSortedParts.erase(0);
    }

    if (mNpc->isEventResultJudgeEnd() && mSortedParts.size() == 0) {
        al::offStageSwitch(this, "SwitchResultOn");
        al::setNerve(this, &End);
    }
}

void FukuwaraiWatcher::exeEnd() {
    if (al::isFirstStep(this)) {
        mNpc->startEndResult();
        al::offStageSwitch(this, "SwitchFukuwaraiOn");
    }

    if (mNpc->isEnd()) {
        mNpc->reset();
        al::enableBgmLineChange(this);
        al::setNerve(this, &Wait);
    }
}
