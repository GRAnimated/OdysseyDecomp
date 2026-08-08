#include "Library/Audio/System/AudioKeeper.h"

#include "Library/Area/AreaObjUtil.h"
#include "Library/Audio/AudioDirector.h"
#include "Library/Bgm/BgmKeeper.h"
#include "Library/LiveActor/ActorAreaFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Player/PlayerUtil.h"
#include "Library/Se/SeKeeper.h"

namespace al {

AudioKeeper::AudioKeeper(const AudioDirector* audioDirector) {
    mAudioMic = audioDirector->getAudioMic();
    mAudioEffectController = audioDirector->getAudioEffectController();
    mAudioEventController = audioDirector->getAudioEventController();
    mAudioRequestKeeperSyncedBgm = audioDirector->getAudioRequestKeeperSyncedBgm();
}

void AudioKeeper::initSeKeeper(const AudioDirector* audioDirector, const char* name,
                               const sead::Vector3f* position,
                               const sead::Matrix34f* emitterMtx, const ModelKeeper* modelKeeper,
                               CameraDirector* cameraDirector) {
    if (name)
        mSeKeeper = SeKeeper::create(audioDirector->getAudioSystemInfo(),
                                     audioDirector->getSeDirector(), name, position, emitterMtx,
                                     modelKeeper, cameraDirector);
}

void AudioKeeper::initBgmKeeper(const AudioDirector* audioDirector, const char* name) {
    mBgmKeeper = BgmKeeper::create(audioDirector->getAudioSystemInfo(),
                                   audioDirector->getBgmDirector(), name);
}

void AudioKeeper::validate() {
    if (mSeKeeper)
        mSeKeeper->validate();
}

void AudioKeeper::invalidate() {
    if (mSeKeeper)
        mSeKeeper->invalidate();
}

void AudioKeeper::startClipped() {
    if (mSeKeeper)
        mSeKeeper->startClipped();
}

void AudioKeeper::endClipped() {
    if (mSeKeeper)
        mSeKeeper->endClipped();
}

void AudioKeeper::appear() {
    if (mSeKeeper)
        mSeKeeper->appear();
}

void AudioKeeper::kill() {
    if (mSeKeeper)
        mSeKeeper->kill();
}

}  // namespace al

namespace alAudioKeeperFunction {

al::AudioKeeper* createAudioKeeper(const al::AudioDirector* audioDirector) {
    return new al::AudioKeeper(audioDirector);
}

// NON_MATCHING: behavior is corpus-complete at 176 bytes versus target 172; the inlined exact AudioKeeper constructor retains one redundant STP XZR,XZR at +0x10 that the target factory eliminates. Next source-level hypothesis: recover the original constructor/factory source relationship that permits this DCE while keeping the standalone constructor and one-argument factory exact.
al::AudioKeeper* createAudioKeeper(const al::AudioDirector* audioDirector, const char* seName,
                                   const char* bgmName) {
    al::AudioKeeper* keeper = new al::AudioKeeper(audioDirector);
    keeper->initSeKeeper(audioDirector, seName, nullptr, nullptr, nullptr, nullptr);
    keeper->initBgmKeeper(audioDirector, bgmName);
    return keeper;
}

}  // namespace alAudioKeeperFunction

namespace al {

AudioKeeper::~AudioKeeper() = default;

AudioGeneralPurposeAreaChecker::AudioGeneralPurposeAreaChecker(const char* areaName)
    : mAreaName(areaName) {}

void AudioGeneralPurposeAreaChecker::init(AreaObjDirector* areaObjDirector) {
    mAreaObjDirector = areaObjDirector;
}

void AudioGeneralPurposeAreaChecker::reset() {
    mIsEnteredArea = false;
    mIsExitedArea = false;
    mIsAreaChanged = false;
    _20 = false;
    mCurArea = nullptr;
    mPrevArea = nullptr;
}

void AudioGeneralPurposeAreaChecker::update() {
    _20 = false;
    mIsEnteredArea = false;
    mIsExitedArea = false;
    mIsAreaChanged = false;

    if (!mPlayerHolder)
        return;

    mPrevArea = mCurArea;
    if (!tryFindAreaObjPlayerOne(const_cast<AreaObj**>(&mCurArea)))
        return;

    if (mPrevArea) {
        mIsEnteredArea = false;
        mIsExitedArea = mCurArea == nullptr;
    } else {
        mIsEnteredArea = mCurArea != nullptr;
        mIsExitedArea = false;
    }

    const AreaObj* curActiveArea = mCurArea && mCurArea->isValid() ? mCurArea : nullptr;
    const AreaObj* prevActiveArea = mPrevArea && mPrevArea->isValid() ? mPrevArea : nullptr;
    mIsAreaChanged = curActiveArea != prevActiveArea;

    bool isCurAreaActive = mCurArea && mCurArea->isValid();
    bool wasCurAreaActive = mIsCurAreaActive;
    if (curActiveArea == prevActiveArea) {
        bool shouldKeepAreaState = isCurAreaActive | !wasCurAreaActive;
        if (!shouldKeepAreaState)
            mIsAreaChanged = true;
    }
    mIsCurAreaActive = isCurAreaActive;

    if (!mCurArea)
        return;

    bool isOneTime = false;
    tryGetAreaObjArg(&isOneTime, mCurArea, "IsOneTime");
    if (isOneTime)
        const_cast<AreaObj*>(mCurArea)->invalidate();
}

bool AudioGeneralPurposeAreaChecker::tryFindAreaObjPlayerOne(AreaObj** area) const {
    const char* areaName = mAreaName;
    const PlayerHolder* playerHolder = mPlayerHolder;
    s32 playerNum = getPlayerNumMax(playerHolder);
    AreaObj* selectedArea = nullptr;
    s32 i = 0;
    bool missedArea = false;

    for (; i < playerNum; i++) {
        LiveActor* player = getPlayerActor(playerHolder, i);
        if (!isAreaTarget(player) || isDead(player))
            continue;

        AreaObj* candidate = tryFindAreaObj(this, areaName, getTrans(player));
        if (!candidate) {
            missedArea = true;
            continue;
        }

        if (!selectedArea || selectedArea->getPriority() < candidate->getPriority())
            selectedArea = candidate;
    }

    *area = selectedArea;
    return missedArea || selectedArea != nullptr;
}


bool AudioGeneralPurposeAreaChecker::isInArea() const {
    if (!mAreaName || !mPlayerHolder || !mAreaObjDirector)
        return false;
    return ::al::tryFindAreaObjPlayerOne(mPlayerHolder, mAreaName) != nullptr;
}

bool AudioGeneralPurposeAreaChecker::isInvaridByOneTime() const {
    if (!mCurArea)
        return false;

    bool isOneTime = false;
    tryGetAreaObjArg(&isOneTime, mCurArea, "IsOneTime");
    if (isOneTime && !mCurArea->isValid())
        return true;
    return false;
}

void AudioGeneralPurposeAreaChecker::setPlayerHolder(const PlayerHolder* playerHolder) {
    mPlayerHolder = playerHolder;
}

s32 AudioGeneralPurposeAreaChecker::getIntArgInCurArea(const char* key) const {
    if (!mCurArea)
        return 0;
    s32 value = 0;
    tryGetAreaObjArg(&value, mCurArea, key);
    return value;
}

f32 AudioGeneralPurposeAreaChecker::getFloatArgInCurArea(const char* key) const {
    if (!mCurArea)
        return 0.0f;
    f32 value = 0.0f;
    tryGetAreaObjArg(&value, mCurArea, key);
    return value;
}

bool AudioGeneralPurposeAreaChecker::getBoolArgInCurArea(const char* key) const {
    if (!mCurArea)
        return false;
    bool value = false;
    tryGetAreaObjArg(&value, mCurArea, key);
    return value;
}

const char* AudioGeneralPurposeAreaChecker::getStringArgInCurArea(const char* key) const {
    if (!mCurArea)
        return nullptr;
    const char* value = nullptr;
    tryGetAreaObjStringArg(&value, mCurArea, key);
    return value;
}

const char* AudioGeneralPurposeAreaChecker::getStringArgInCurAreaWithAreaCheck(
    const char* key) const {
    if (!key)
        return nullptr;
    const AreaObj* area = ::al::tryFindAreaObjPlayerOne(mPlayerHolder, mAreaName);
    if (!area)
        return nullptr;
    const char* value = nullptr;
    tryGetAreaObjStringArg(&value, area, key);
    return value;
}

s32 AudioGeneralPurposeAreaChecker::getIntArgInPastArea(const char* key) const {
    if (!mPrevArea)
        return 0;
    s32 value = 0;
    tryGetAreaObjArg(&value, mPrevArea, key);
    return value;
}

const char* AudioGeneralPurposeAreaChecker::getStringArgInPastArea(const char* key) const {
    if (!mPrevArea)
        return nullptr;
    const char* value = nullptr;
    tryGetAreaObjStringArg(&value, mPrevArea, key);
    return value;
}

bool AudioGeneralPurposeAreaChecker::tryGetIntArgInCurArea(s32* out, const char* key) const {
    if (!mCurArea)
        return false;
    return tryGetAreaObjArg(out, mCurArea, key);
}

bool AudioGeneralPurposeAreaChecker::tryGetStringArgInCurArea(const char** out,
                                                               const char* key) const {
    if (!mCurArea)
        return false;
    return tryGetAreaObjStringArg(out, mCurArea, key);
}

}  // namespace al
