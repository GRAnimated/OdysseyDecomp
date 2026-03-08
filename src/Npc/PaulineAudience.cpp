#include "Npc/PaulineAudience.h"

#include <cstring>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorResourceFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Shadow/ActorShadowUtil.h"
#include "Library/Yaml/ByamlIter.h"

#include "Npc/BgmAnimeSynchronizer.h"
#include "Npc/CityManRhythmInfo.h"
#include "Npc/NpcStateReaction.h"
#include "Util/SensorMsgFunction.h"

static const char* sAnimNames[] = {
    "Excited",      "ExcitedHandclap",    "ExcitedMax", "ExcitedMaxJump",
    "HandClapWait", "HandClapWaitMiddle", "WaitShop",
};

namespace {
NERVE_IMPL(PaulineAudience, Dance);
NERVE_IMPL(PaulineAudience, Sabi);
NERVE_IMPL(PaulineAudience, Reaction);
NERVES_MAKE_NOSTRUCT(PaulineAudience, Dance, Sabi, Reaction);
}  // namespace

PaulineAudience::PaulineAudience(const char* name) : al::LiveActor(name) {}

void PaulineAudience::init(const al::ActorInitInfo& info) {
    al::initMapPartsActor(this, info, nullptr);
    al::initNerve(this, &Dance, 1);

    mNpcStateReaction = NpcStateReaction::createForHuman(this, nullptr);
    mNpcStateReaction->_29 = true;
    al::initNerveState(this, mNpcStateReaction, &Reaction, u8"リアクション");

    {
        s32 initialAnim = 0;
        if (al::tryGetArg(&initialAnim, info, "PaulineAudienceInitialAnim") &&
            (u32)initialAnim <= 6)
            mInitialAnimIndex = initialAnim;
    }

    {
        const char* mtpAnim = nullptr;
        if (al::tryGetStringArg(&mtpAnim, info, "MtpAnim")) {
            if (strcmp(mtpAnim, "None") != 0)
                al::startMtpAnim(this, mtpAnim);
        }
    }

    {
        const char* mclAnim = nullptr;
        if (al::tryGetStringArg(&mclAnim, info, "MclAnim"))
            al::startMclAnim(this, mclAnim);
    }

    {
        al::ByamlIter iter;
        if (al::tryGetActorInitFileIter(&iter, this, "BgmRhythmSyncInfo", nullptr)) {
            mBgmAnimeSynchronizer = BgmAnimeSynchronizer::tryCreate(this, iter);
            if (mBgmAnimeSynchronizer) {
                if (!al::isInitializedBgmKeeper(this))
                    al::initActorBgmKeeper(this, info, nullptr);
            }
        }
    }

    auto* rhythmInfo = new CityManRhythmInfo(
        this, al::getModelResourceYaml(this, "DanceAnimInfo", nullptr), false, 0.0f);
    mRhythmInfo = rhythmInfo;

    al::offDepthShadowModel(this);

    {
        s32 lodLevel = 0;
        if (al::tryGetArg(&lodLevel, info, "ForceLODLevel"))
            al::forceLodLevel(this, lodLevel);
    }

    makeActorAlive();
}

// NON_MATCHING: comparison encoding differs (cmp #1;b.gt vs cmp #2;b.ge)
void PaulineAudience::control() {
    if (!mRhythmInfo)
        return;

    mRhythmInfo->update(_12c);

    if (mRhythmInfo->_35 || _12c) {
        _12c = false;
        s32 count = mRhythmInfo->mAnimIndex;
        if (count >= 1) {
            s32 idx = count - 1;
            s32 animId = mRhythmInfo->getAnimId(idx);
            f32 currentBeat = mRhythmInfo->mCurrentBeat;
            f32 animBeat = mRhythmInfo->getAnimBeat(idx);
            mSabiFrame = (currentBeat - animBeat) * 3600.0f / 204.0f;
            if (animId > 1)
                al::setNerve(this, &Sabi);
            return;
        }
    }

    mSabiFrame = -1.0f;
    if (mRhythmInfo->mAnimId >= 2)
        al::setNerve(this, &Sabi);
}

// NON_MATCHING: comparison encoding differs (cmp #1;b.gt vs cmp #2;b.ge)
void PaulineAudience::forceControlForDance() {
    s32 count = mRhythmInfo->mAnimIndex;

    if (count >= 1) {
        s32 idx = count - 1;
        s32 animId = mRhythmInfo->getAnimId(idx);
        f32 currentBeat = mRhythmInfo->mCurrentBeat;
        f32 animBeat = mRhythmInfo->getAnimBeat(idx);
        mSabiFrame = (currentBeat - animBeat) * 3600.0f / 204.0f;
        if (animId > 1)
            al::setNerve(this, &Sabi);
        return;
    }

    mSabiFrame = -1.0f;
    if (mRhythmInfo->mAnimId >= 2)
        al::setNerve(this, &Sabi);
}

void PaulineAudience::controlForDance() {
    mSabiFrame = -1.0f;
    if (mRhythmInfo->mAnimId >= 2)
        al::setNerve(this, &Sabi);
}

void PaulineAudience::endClipped() {
    _12c = true;
    al::LiveActor::endClipped();
    if (mBgmAnimeSynchronizer)
        mBgmAnimeSynchronizer->_28 = true;
}

void PaulineAudience::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isSensorEye(self))
        return;
    if (!rs::sendMsgPushToPlayer(other, self))
        al::sendMsgPush(other, self);
}

bool PaulineAudience::receiveMsg(const al::SensorMsg* msg, al::HitSensor* self,
                                 al::HitSensor* other) {
    if (rs::isMsgPlayerDisregardHomingAttack(msg))
        return true;
    if (mNpcStateReaction->receiveMsg(msg, self, other)) {
        al::setNerve(this, &Reaction);
        return true;
    }
    return false;
}

void PaulineAudience::exeDance() {
    if (al::isFirstStep(this)) {
        al::startAction(this, sAnimNames[mInitialAnimIndex]);
        al::validateClipping(this);
    }
    if (mBgmAnimeSynchronizer)
        mBgmAnimeSynchronizer->trySyncBgm();
}

void PaulineAudience::exeSabi() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "MarioJump");
        if (mSabiFrame > 0.0f) {
            al::getActionFrameMax(this, "MarioJump");
            al::setActionFrame(this, mSabiFrame);
        }
        al::invalidateClipping(this);
    }
    if (al::isActionEnd(this))
        al::setNerve(this, &Dance);
}

void PaulineAudience::exeReaction() {
    if (al::isFirstStep(this))
        al::invalidateClipping(this);
    if (al::updateNerveState(this))
        al::setNerve(this, &Dance);
}
