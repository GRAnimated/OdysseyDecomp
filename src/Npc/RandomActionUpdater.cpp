#include "Npc/RandomActionUpdater.h"

#include <math/seadVector.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/TalkNpcActionAnimInfo.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerUtil.h"

namespace {
using namespace al;

NERVE_IMPL(RandomActionUpdater, Wait);
NERVE_IMPL(RandomActionUpdater, PlayOneTime);
NERVE_IMPL(RandomActionUpdater, BalloonAction);
NERVE_IMPL(RandomActionUpdater, RandomOneTime);
NERVE_IMPL(RandomActionUpdater, RandomLoop);
NERVES_MAKE_STRUCT(RandomActionUpdater, Wait, PlayOneTime, BalloonAction, RandomOneTime,
                   RandomLoop);

void tryGetRandomActionName(sead::BufferedSafeStringBase<char>* outName, const al::LiveActor* actor,
                            const TalkNpcActionAnimInfo* info) {
    if (!al::isSklAnimPlaying(actor, 0))
        return;

    al::StringTmp<64> result("");

    if (info->isSelectedInitWaitAction() &&
        rs::isPlayingNpcAction(actor, info->getWaitActionName()) && info->_50 >= 1) {
        result.format(info->getAnyRandomActionName());
    } else {
        const char* animName = al::getPlayingSklAnimName(actor, 0);
        al::StringTmp<64> randomName("%sRandom", animName);
        if (rs::isExistNpcAction(actor, randomName.cstr()))
            result.format("%sRandom", animName);
    }

    if (*result.cstr() != sead::SafeStringBase<char>::cNullChar)
        rs::makeNpcActionName(outName, actor, result.cstr());
}

}  // namespace

// NON_MATCHING: field init ordering and sead::FixedSafeString buffer clearing differs
RandomActionUpdater::RandomActionUpdater(al::LiveActor* actor, const TalkNpcActionAnimInfo* info)
    : al::NerveExecutor("ランダムアクション制御"), mActor(actor), mActionAnimInfo(info),
      _20(nullptr), _28(nullptr), _30(nullptr), _90(nullptr), _98(false), _99(false), _9c(0),
      _a0(0.2f) {
    initNerve(&NrvRandomActionUpdater.Wait, 0);
}

void RandomActionUpdater::initBalloonAction(const char* actionName) {
    rs::makeNpcActionName(&mBalloonActionName, mActor, actionName);
}

void RandomActionUpdater::startActionOneTime(const char* actionName) {
    if (al::isNerve(this, &NrvRandomActionUpdater.Wait))
        _20 = al::getActionName(mActor);
    rs::startNpcAction(mActor, actionName);
    _30 = al::getActionName(mActor);
    al::setNerve(this, &NrvRandomActionUpdater.PlayOneTime);
}

void RandomActionUpdater::tryStartWaitActionIfNotPlaying() {
    if (al::isNerve(this, &NrvRandomActionUpdater.Wait)) {
        if (!rs::isPlayingNpcAction(mActor, mActionAnimInfo->getWaitActionName()))
            rs::startNpcAction(mActor, mActionAnimInfo->getWaitActionName());
    }
}

void RandomActionUpdater::update() {
    updateNerve();
}

void RandomActionUpdater::forceEnd() {
    if (!al::isNerve(this, &NrvRandomActionUpdater.Wait))
        al::setNerve(this, &NrvRandomActionUpdater.Wait);
}

void RandomActionUpdater::restart() {
    al::setNerve(this, &NrvRandomActionUpdater.Wait);
}

bool RandomActionUpdater::isPlayingBalloonAction() const {
    return al::isNerve(this, &NrvRandomActionUpdater.BalloonAction);
}

// NON_MATCHING: shouldUpdate boolean optimization, cstr() virtual dispatch, regalloc
void RandomActionUpdater::exeWait() {
    if (al::isFirstStep(this)) {
        _28 = nullptr;
        _30 = nullptr;
        _20 = nullptr;
    }

    bool isPlaying = al::isSklAnimPlaying(mActor, 0);

    bool shouldUpdate = al::isFirstStep(this);
    if (!shouldUpdate) {
        bool wasPlaying = al::isSklAnimPlaying(mActor, 0);
        if (!_90)
            shouldUpdate = wasPlaying;
        else
            shouldUpdate =
                !wasPlaying || !al::isEqualString(_90, al::getPlayingSklAnimName(mActor, 0));
    }

    if (shouldUpdate) {
        if (isPlaying) {
            _90 = al::getPlayingSklAnimName(mActor, 0);
            _98 = al::isSklAnimOneTime(mActor, 0);
            al::StringTmp<64> randomName("");
            tryGetRandomActionName(&randomName, mActor, mActionAnimInfo);
            _99 = *randomName.cstr() != sead::SafeStringBase<char>::cNullChar;
        } else {
            _90 = nullptr;
            _98 = false;
            _99 = false;
        }
    }

    if (al::isFirstStep(this))
        return;

    if (_99 && !_98) {
        al::LiveActor* actor = mActor;
        f32 frame = al::getActionFrame(actor);
        f32 rate = al::getActionFrameRate(actor);
        f32 nextFrame = frame + rate;
        const char* actionName = al::getActionName(actor);
        f32 maxFrame = al::getActionFrameMax(actor, actionName);
        f32 wrapped = al::modf(nextFrame + maxFrame, maxFrame);
        f32 wrappedClean = wrapped + 0.0f;
        f32 currentFrame = al::getActionFrame(actor);
        if (!(wrappedClean >= currentFrame)) {
            if (al::getRandom() < _a0) {
                al::StringTmp<64> randomName("");
                tryGetRandomActionName(&randomName, mActor, mActionAnimInfo);
                if (*randomName.cstr() == sead::SafeStringBase<char>::cNullChar) {
                    _99 = false;
                    return;
                }
                _20 = al::getActionName(mActor);
                al::startAction(mActor, randomName.cstr());
                _28 = al::getActionName(mActor);
                if (al::isSklAnimOneTime(mActor, randomName.cstr()))
                    al::setNerve(this, &NrvRandomActionUpdater.RandomOneTime);
                else
                    al::setNerve(this, &NrvRandomActionUpdater.RandomLoop);
                return;
            }
        }
    }

    if (*mBalloonActionName.cstr() != sead::SafeStringBase<char>::cNullChar &&
        !mActionAnimInfo->_61) {
        if (_9c >= 1) {
            if (rs::isNearPlayerH(mActor, 1000.0f))
                _9c = 600;
            else
                _9c--;
        } else if (rs::isSuccessNpcEventBalloonMessage(mActor)) {
            if (rs::isNearPlayerH(mActor, 500.0f)) {
                sead::Vector3f frontDir = {0.0f, 0.0f, 0.0f};
                sead::Vector3f playerDir = {0.0f, 0.0f, 0.0f};
                al::calcFrontDir(&frontDir, mActor);
                const sead::Vector3f& playerPos = rs::getPlayerPos(mActor);
                al::calcDirToActorH(&playerDir, mActor, playerPos);
                if (al::calcAngleDegree(frontDir, playerDir) < 30.0f) {
                    _20 = al::getActionName(mActor);
                    al::setNerve(this, &NrvRandomActionUpdater.BalloonAction);
                }
            }
        }
    }
}

// NON_MATCHING: nerve address load scheduling
void RandomActionUpdater::exeBalloonAction() {
    if (al::isFirstStep(this))
        rs::startNpcAction(mActor, mBalloonActionName.cstr());
    if (al::isActionPlaying(mActor, mBalloonActionName.cstr())) {
        if (!al::isActionEnd(mActor))
            return;
        _9c = 600;
        if (_20)
            rs::startNpcAction(mActor, _20);
    } else {
        _9c = 600;
    }
    al::setNerve(this, &NrvRandomActionUpdater.Wait);
}

void RandomActionUpdater::exePlayOneTime() {
    if (al::isActionPlaying(mActor, _30)) {
        if (!al::isActionEnd(mActor))
            return;
        if (_20)
            rs::startNpcAction(mActor, _20);
    }
    al::setNerve(this, &NrvRandomActionUpdater.Wait);
}

void RandomActionUpdater::exeRandomOneTime() {
    if (al::isActionPlaying(mActor, _28)) {
        if (!al::isActionEnd(mActor))
            return;
        if (_20)
            rs::startNpcAction(mActor, _20);
    }
    al::setNerve(this, &NrvRandomActionUpdater.Wait);
}

// NON_MATCHING: b.pl vs b.ge encoding for fcmp comparison
void RandomActionUpdater::exeRandomLoop() {
    if (!al::isActionPlaying(mActor, _28)) {
        al::setNerve(this, &NrvRandomActionUpdater.Wait);
        return;
    }
    al::LiveActor* actor = mActor;
    f32 frame = al::getActionFrame(actor);
    f32 rate = al::getActionFrameRate(actor);
    f32 nextFrame = frame + rate;
    const char* actionName = al::getActionName(actor);
    f32 maxFrame = al::getActionFrameMax(actor, actionName);
    f32 wrapped = al::modf(nextFrame + maxFrame, maxFrame);
    f32 wrappedClean = wrapped + 0.0f;
    f32 currentFrame = al::getActionFrame(actor);
    if (!(wrappedClean >= currentFrame)) {
        if (al::getRandom() < 0.4f) {
            if (_20)
                rs::startNpcAction(mActor, _20);
            al::setNerve(this, &NrvRandomActionUpdater.Wait);
        }
    }
}
