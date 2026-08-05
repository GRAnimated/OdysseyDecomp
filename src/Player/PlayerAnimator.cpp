#include "Player/PlayerAnimator.h"

#include "Library/Action/ActorActionKeeper.h"
#include "Library/Base/StringUtil.h"
#include "Library/Effect/EffectSystemInfo.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Obj/ActorDitherAnimator.h"

#include "Player/PlayerAnimFrameCtrl.h"
#include "Player/PlayerModelHolder.h"

namespace {

const char* const sPlayerParts[] = {"顔", "目", "頭", "左手", "右手"};
const char* const sDeadAnims[] = {"Dead01", "Dead02"};

void setPartsAnimFrameLocal(al::LiveActor* actor, const char* actionName, f32 frame) {
    if (!al::isActionPlaying(actor, actionName))
        return;

    if (al::isSklAnimExist(actor, actionName))
        al::setSklAnimBlendFrameAll(actor, frame, true);
    if (al::isMtpAnimExist(actor, actionName))
        al::setMtpAnimFrame(actor, frame);
    if (al::isMtsAnimExist(actor, actionName))
        al::setMtsAnimFrame(actor, frame);
    if (al::isVisAnimExist(actor, actionName))
        al::setVisAnimFrameForAction(actor, frame);
}

void setPartsAnimRateLocal(al::LiveActor* actor, const char* actionName, f32 rate) {
    if (!al::isActionPlaying(actor, actionName))
        return;

    if (al::isSklAnimExist(actor, actionName))
        al::setSklAnimBlendFrameRateAll(actor, rate, true);
    if (al::isMtpAnimExist(actor, actionName))
        al::setMtpAnimFrameRate(actor, rate);
    if (al::isMtsAnimExist(actor, actionName))
        al::setMtsAnimFrameRate(actor, rate);
    if (al::isVisAnimExist(actor, actionName))
        al::setVisAnimFrameRateForAction(actor, rate);
}

void restoreEyeAnim(al::LiveActor* player) {
    const char* actionName = al::getActionName(player);
    f32 frame = al::getActionFrame(player);
    f32 rate = al::getActionFrameRate(player);
    al::LiveActor* eye = al::tryGetSubActor(player, "目");
    if (!eye)
        return;

    if (al::tryStartAction(eye, actionName)) {
        setPartsAnimRateLocal(eye, actionName, rate);
        setPartsAnimFrameLocal(eye, actionName, frame);
    } else {
        al::startAction(eye, "Wait");
    }

    if (al::isMtsAnimPlaying(eye, "EyeMove"))
        al::startMtsAnim(eye, "EyeReset");
}

}  // namespace

template <s32 blendType>
__attribute__((always_inline)) inline void PlayerAnimator::startUpperBodyAnimCommon(
    const sead::SafeString& animName) {
    PlayerModelHolder* modelHolder = mModelHolder;
    al::LiveActor* player = modelHolder->getActor();
    sead::FixedSafeString<64>* upperBodyAnim = &mCurUpperBodyAnim;
    al::LiveActor* normal = modelHolder->findModelActor("Normal");
    upperBodyAnim->format("%s%s", animName.cstr(), modelHolder->get_18());
    if (!al::isExistAction(normal, upperBodyAnim->cstr()))
        upperBodyAnim->format("%s", animName.cstr());
    al::startPartialSklAnim(player, upperBodyAnim->cstr(), 0, blendType, 0);
    startPartsPartialAnim(upperBodyAnim->cstr());
    _1a3 = true;
    mIsUpperBodyAnimHeadVisKeep = false;
}

PlayerAnimator::PlayerAnimator(const PlayerModelHolder* modelHolder,
                               al::ActorDitherAnimator* ditherAnimator)
    : mModelHolder(const_cast<PlayerModelHolder*>(modelHolder)), mPlayerDeco(nullptr),
      mPlayer(nullptr), mAnimFrameCtrl(new PlayerAnimFrameCtrl), mCurAnim(""), mCurSubAnim(""),
      mCurUpperBodyAnim(""), _128(""), mDitherAnim(ditherAnimator), mEyeControlFrame(0.0f),
      mEndEyeControlAnimDelay(0), mRunStartAnimRate(0.0f), _19c(0), mIsNeedFullFaceAnim(false),
      mIsSubAnimPlaying(false), _1a3(false), mIsUpperBodyAnimHeadVisKeep(false), _1a5(false),
      _1a6(false), mIsSubAnimOnlyAir(false) {
    mSklAnimBlendWeights = new f32[6];
    mPlayer = modelHolder->getActor();
    mPlayerDeco = al::tryGetSubActor(modelHolder->findModelActor("Normal"), "");
    startAnim("Wait");
}

void PlayerAnimator::startAnim(const sead::SafeString& animName) {
    PlayerModelHolder* modelHolder = mModelHolder;
    al::LiveActor* player = modelHolder->getActor();
    sead::FixedSafeString<64>* currentAnim = &mCurAnim;
    al::LiveActor* normal = modelHolder->findModelActor("Normal");
    currentAnim->format("%s%s", animName.cstr(), modelHolder->get_18());
    if (!al::isExistAction(normal, currentAnim->cstr()))
        currentAnim->format("%s", animName.cstr());

    mAnimFrameCtrl->startAction(player, *currentAnim);
    _1a1 = false;
    if (!mIsSubAnimPlaying)
        startAnimCommon(*currentAnim);
    mAnimFrameCtrl->setRate(1.0f);
    if (!mIsSubAnimPlaying)
        setAnimRateCommon(1.0f);
}

void PlayerAnimator::updateAnimFrame() {
    if (mIsSubAnimPlaying)
        mAnimFrameCtrl->update();
    else
        mAnimFrameCtrl->updateSync(mModelHolder->getActor());
}

void PlayerAnimator::updateModel() {
    al::LiveActor* current = mModelHolder->getActor();
    if (mPlayer != current) {
        copyAnim();
        mPlayer = current;
    }
}

void PlayerAnimator::copyAnim() {
    copyAnimLocal();
    if (_1a3) {
        al::clearPartialSklAnim(mPlayer, 0);
        _1a3 = false;
    }
    resetModelAlpha();
}

void PlayerAnimator::startAnimCommon(const sead::SafeString& animName) {
    al::startAction(mModelHolder->getActor(), animName.cstr());
    startPartsAnim(animName.cstr());
}

void PlayerAnimator::setAnimRate(f32 rate) {
    mAnimFrameCtrl->setRate(rate);
    if (!mIsSubAnimPlaying)
        setAnimRateCommon(rate);
}

void PlayerAnimator::startAnimSpinAttack(const sead::SafeString& animName) {
    PlayerModelHolder* modelHolder = mModelHolder;
    sead::FixedSafeString<64>* spinAnim = &_128;
    al::LiveActor* normal = modelHolder->findModelActor("Normal");
    spinAnim->format("%s%s", animName.cstr(), modelHolder->get_18());
    if (!al::isExistAction(normal, spinAnim->cstr()))
        spinAnim->format("%s", animName.cstr());
    startAnim(animName);
}

void PlayerAnimator::setAnimRateCommon(f32 rate) {
    if (al::isSklAnimPlaying(mModelHolder->getActor(), 0))
        al::setSklAnimBlendFrameRateAll(mModelHolder->getActor(), rate, true);
    if (al::isVisAnimPlayingForAction(mModelHolder->getActor()))
        al::setVisAnimFrameRateForAction(mModelHolder->getActor(), rate);
    setPartsAnimRate(rate, mCurAnim.cstr());
}

void PlayerAnimator::setAnimFrame(f32 frame) {
    mAnimFrameCtrl->setFrame(frame);
    if (!mIsSubAnimPlaying)
        setAnimFrameCommon(frame);
}

void PlayerAnimator::setAnimFrameCommon(f32 frame) {
    if (al::isSklAnimExist(mModelHolder->getActor(), mCurAnim.cstr())) {
        al::setSklAnimBlendFrameAll(mModelHolder->getActor(), frame, true);
    } else if (al::isVisAnimExist(mModelHolder->getActor(), mCurAnim.cstr())) {
        f32 maxFrame = al::getVisAnimFrameMaxForAction(mModelHolder->getActor());
        if (maxFrame > frame)
            maxFrame = frame;
        al::setVisAnimFrameForAction(mModelHolder->getActor(), maxFrame);
    }
    setPartsAnimFrame(frame, mCurAnim.cstr());
}

bool PlayerAnimator::isAnimEnd() const {
    return mAnimFrameCtrl->isActionEnd();
}

bool PlayerAnimator::isAnim(const sead::SafeString& animName) const {
    if (al::isEqualString(animName.cstr(), mCurAnim.cstr()))
        return true;
    return al::isEqualString(animName.cstr(), mAnimFrameCtrl->getActionName());
}

bool PlayerAnimator::isCurrentAnimOneTime() const {
    return !mAnimFrameCtrl->isActionRepeat();
}

f32 PlayerAnimator::getAnimFrame() const {
    return mAnimFrameCtrl->getCurrentFrame();
}

f32 PlayerAnimator::getAnimFrameMax() const {
    return mAnimFrameCtrl->getActionFrameMax();
}

f32 PlayerAnimator::getAnimFrameRate() const {
    return mAnimFrameCtrl->getRate();
}

void PlayerAnimator::clearInterpolation() {
    if (!mIsSubAnimPlaying)
        al::clearSklAnimInterpole(mModelHolder->getActor());
}

void PlayerAnimator::startSubAnim(const sead::SafeString& animName) {
    PlayerModelHolder* modelHolder = mModelHolder;
    mIsSubAnimPlaying = true;
    mIsSubAnimOnlyAir = false;
    sead::FixedSafeString<64>* subAnim = &mCurSubAnim;
    al::LiveActor* normal = modelHolder->findModelActor("Normal");
    subAnim->format("%s%s", animName.cstr(), modelHolder->get_18());
    if (!al::isExistAction(normal, subAnim->cstr()))
        subAnim->format("%s", animName.cstr());

    startAnimCommon(*subAnim);
}

void PlayerAnimator::startSubAnimOnlyAir(const sead::SafeString& animName) {
    startSubAnim(animName);
    mIsSubAnimOnlyAir = true;
}

void PlayerAnimator::endSubAnim() {
    mIsSubAnimPlaying = false;
    mIsSubAnimOnlyAir = false;
    if (al::isEqualString(mCurAnim.cstr(), mCurSubAnim.cstr()))
        return;

    const char* actionName = mAnimFrameCtrl->getActionName();
    startAnimCommon(actionName);
    setAnimRate(mAnimFrameCtrl->getRate());
    setAnimFrame(mAnimFrameCtrl->getCurrentFrame());
    applyBlendWeight();
}

void PlayerAnimator::applyBlendWeight() {
    if (!_1a1)
        return;

    al::LiveActor* player = mModelHolder->getActor();
    al::setSklAnimBlendWeightSixfold(player, mSklAnimBlendWeights[0], mSklAnimBlendWeights[1],
                                     mSklAnimBlendWeights[2], mSklAnimBlendWeights[3],
                                     mSklAnimBlendWeights[4], mSklAnimBlendWeights[5]);
    const char* actionName = al::getActionName(player);
    if (_1a3) {
        al::LiveActor* face = al::tryGetSubActor(player, "顔");
        if (face && al::isActionPlaying(face, actionName) && al::isSklAnimExist(face))
            al::setSklAnimBlendWeightSixfold(face, mSklAnimBlendWeights[0], mSklAnimBlendWeights[1],
                                             mSklAnimBlendWeights[2], mSklAnimBlendWeights[3],
                                             mSklAnimBlendWeights[4], mSklAnimBlendWeights[5]);
        al::LiveActor* eye = al::tryGetSubActor(player, "目");
        if (eye && al::isActionPlaying(eye, actionName) && al::isSklAnimExist(eye))
            al::setSklAnimBlendWeightSixfold(eye, mSklAnimBlendWeights[0], mSklAnimBlendWeights[1],
                                             mSklAnimBlendWeights[2], mSklAnimBlendWeights[3],
                                             mSklAnimBlendWeights[4], mSklAnimBlendWeights[5]);
        return;
    }
    for (s32 i = 0; i != 5; i++) {
        al::LiveActor* actor = al::tryGetSubActor(player, sPlayerParts[i]);
        if (actor && al::isActionPlaying(actor, actionName) && al::isSklAnimExist(actor))
            al::setSklAnimBlendWeightSixfold(
                actor, mSklAnimBlendWeights[0], mSklAnimBlendWeights[1], mSklAnimBlendWeights[2],
                mSklAnimBlendWeights[3], mSklAnimBlendWeights[4], mSklAnimBlendWeights[5]);
    }
}

void PlayerAnimator::setSubAnimFrame(f32 frame) {
    setAnimFrameCommon(frame);
}

void PlayerAnimator::setSubAnimRate(f32 rate) {
    setAnimRateCommon(rate);
}

bool PlayerAnimator::isSubAnimEnd() const {
    if (!mIsSubAnimPlaying)
        return true;
    al::LiveActor* player = mModelHolder->getActor();
    if (al::isSklAnimExist(player, al::getActionName(player)))
        return al::isActionEnd(player);
    return al::isVisAnimEnd(player);
}

bool PlayerAnimator::isSubAnim(const sead::SafeString& animName) const {
    return mIsSubAnimPlaying && al::isEqualString(animName.cstr(), mCurSubAnim.cstr());
}

f32 PlayerAnimator::getSubAnimFrame() const {
    return al::getActionFrame(mModelHolder->getActor());
}

f32 PlayerAnimator::getSubAnimFrameMax() const {
    return al::getActionFrameMax(mModelHolder->getActor());
}

bool PlayerAnimator::isUpperBodyAnimAttached() const {
    return al::isPartialSklAnimAttached(mModelHolder->getActor(), 0);
}

bool PlayerAnimator::isUpperBodyAnimEnd() const {
    return al::isPartialSklAnimEnd(mModelHolder->getActor(), 0);
}

bool PlayerAnimator::isUpperBodyAnim(const sead::SafeString& animName) const {
    return al::isPartialSklAnimPlaying(mModelHolder->getActor(), animName.cstr(), 0);
}

void PlayerAnimator::startUpperBodyAnim(const sead::SafeString& animName) {
    startUpperBodyAnimCommon<0>(animName);
}

void PlayerAnimator::startPartsPartialAnim(const sead::SafeString& animName) {
    al::LiveActor* player = mModelHolder->getActor();
    for (const char* part : {"頭", "左手", "右手"}) {
        al::LiveActor* actor = al::tryGetSubActor(player, part);
        if (actor)
            al::tryStartAction(actor, animName.cstr());
    }
}

void PlayerAnimator::startUpperBodyAnimSubParts(const sead::SafeString& animName) {
    startUpperBodyAnimCommon<1>(animName);
}

void PlayerAnimator::startUpperBodyAnimAndHeadVisKeep(const sead::SafeString& animName) {
    startUpperBodyAnim(animName);
    mIsUpperBodyAnimHeadVisKeep = true;
}

void PlayerAnimator::clearUpperBodyAnim() {
    al::clearPartialSklAnimWithInterpolate(mModelHolder->getActor(), 0, 5);
    al::LiveActor* player = mModelHolder->getActor();
    const char* actionName = al::getActionName(player);
    f32 frame = al::getActionFrame(player);
    f32 rate = al::getActionFrameRate(player);
    bool isSameAction = al::isEqualString(actionName, _128.cstr());

    for (const char* part : {"頭", "左手", "右手"}) {
        al::LiveActor* actor = al::tryGetSubActor(player, part);
        if (!actor || (isSameAction && al::isEqualString("頭", part)))
            continue;
        if (!al::tryStartAction(actor, actionName))
            al::startAction(actor, "Wait");
        setPartsAnimFrameLocal(actor, actionName, frame);
        setPartsAnimRateLocal(actor, actionName, rate);
    }

    if (mIsUpperBodyAnimHeadVisKeep) {
        al::LiveActor* head = al::tryGetSubActor(player, "頭");
        if (head) {
            al::startVisAnimForAction(head, mCurUpperBodyAnim.cstr());
            al::setVisAnimFrameForAction(head, al::getVisAnimFrameMaxForAction(head));
        }
    }
    _1a3 = false;
    mIsUpperBodyAnimHeadVisKeep = false;
}

void PlayerAnimator::setBlendWeight(f32 weight0, f32 weight1, f32 weight2, f32 weight3, f32 weight4,
                                    f32 weight5) {
    mSklAnimBlendWeights[0] = weight0;
    mSklAnimBlendWeights[1] = weight1;
    mSklAnimBlendWeights[2] = weight2;
    mSklAnimBlendWeights[3] = weight3;
    mSklAnimBlendWeights[4] = weight4;
    mSklAnimBlendWeights[5] = weight5;
    _1a1 = true;
    if (!mIsSubAnimPlaying)
        applyBlendWeight();
}

f32 PlayerAnimator::getBlendWeight(s32 index) {
    return mSklAnimBlendWeights[index];
}

void PlayerAnimator::startAnimDead() {
    if (al::getRandom(0, 20))
        startAnim(sDeadAnims[al::getRandom(0, 2)]);
    else
        startAnim("Dead04");
}

void PlayerAnimator::startPress() {
    setAnimRate(0.0f);
    mModelHolder->getActor()->getActorActionKeeper()->tryStartActionNoAnim("Press");
}

void PlayerAnimator::forceCapOn() {
    al::LiveActor* head = al::tryGetSubActor(mModelHolder->findModelActor("Normal"), "頭");
    if (head)
        al::startVisAnimForAction(head, "CapOn");
}

void PlayerAnimator::forceCapOff() {
    al::LiveActor* head = al::tryGetSubActor(mModelHolder->findModelActor("Normal"), "頭");
    if (head)
        al::startVisAnimForAction(head, "CapOff");
}

f32 PlayerAnimator::getModelAlpha() const {
    return mDitherAnim->getDitherAlpha();
}

void PlayerAnimator::updateModelAlpha() {
    f32 alpha;
    if (_19c) {
        _19c = al::converge(_19c, 0, 1);
        mDitherAnim->reset();
        alpha = 1.0f;
    } else {
        mDitherAnim->update();
        alpha = mDitherAnim->getDitherAlpha();
    }
    setModelAlpha(alpha);
}

void PlayerAnimator::setModelAlpha(f32 alpha) {
    al::LiveActor* player = mModelHolder->getActor();
    al::setModelAlphaMask(player, alpha);
    for (const char* part : sPlayerParts) {
        al::LiveActor* actor = al::tryGetSubActor(player, part);
        if (actor)
            al::setModelAlphaMask(actor, al::getModelAlphaMask(player));
    }
}

void PlayerAnimator::resetModelAlpha() {
    mDitherAnim->reset();
    setModelAlpha(1.0f);
}

void PlayerAnimator::endDemoInvalidateModelAlpha() {
    s32 value = _19c;
    if (value <= 1)
        value = 1;
    _19c = value;
}

void PlayerAnimator::startSnapShotMode() {
    mDitherAnim->setClippingJudgeDistanceParam("SnapShotMode");
}

void PlayerAnimator::endSnapShotMode() {
    mDitherAnim->resetClippingJudgeDistanceParam();
}

void PlayerAnimator::startEyeControlAnim(bool isReset) {
    _1a5 = true;
    _1a6 = isReset;
    mEndEyeControlAnimDelay = 0;
}

void PlayerAnimator::endEyeControlAnim(s32 delay) {
    _1a5 = false;
    _1a6 = false;
    mEndEyeControlAnimDelay = delay;
    if (delay == 0)
        restoreEyeAnim(mModelHolder->getActor());
}

void PlayerAnimator::clearEndEyeControlAnimDelay() {
    if (mEndEyeControlAnimDelay >= 1) {
        mEndEyeControlAnimDelay = 0;
        restoreEyeAnim(mModelHolder->getActor());
    }
}

void PlayerAnimator::updateEyeControlAnim() {
    if (_1a5) {
        f32 eyeControlFrame = mEyeControlFrame;
        al::LiveActor* eye = al::tryGetSubActor(mModelHolder->getActor(), "目");
        if (eye && al::isMtsAnimExist(eye, "EyeMove"))
            al::startMtsAnimAndSetFrameAndStop(eye, "EyeMove", eyeControlFrame);
    } else if (mEndEyeControlAnimDelay >= 1) {
        mEndEyeControlAnimDelay = al::converge(mEndEyeControlAnimDelay, 0, 1);
        if (mEndEyeControlAnimDelay == 0)
            restoreEyeAnim(mModelHolder->getActor());
    }
}

void PlayerAnimator::startRightHandAnim(const char* animName) {
    al::LiveActor* hand = al::tryGetSubActor(mModelHolder->getActor(), "右手");
    if (hand)
        al::startAction(hand, animName);
}

void PlayerAnimator::overwrideYoshiEatVis() {
    al::tryStartVisAnimIfNotPlayingForAction(mModelHolder->getActor(), "Eat");
}

void PlayerAnimator::restartYoshiActionVis() {
    if (mCurAnim.getStringTop()[0] == sead::SafeStringBase<char>::cNullChar)
        return;

    al::LiveActor* player = mModelHolder->getActor();
    al::startVisAnimForAction(player, "Wait");
    if (al::tryStartVisAnimIfExistForAction(player, mCurAnim.cstr())) {
        f32 frame = al::getActionFrameMax(player);
        f32 visFrame = al::getVisAnimFrameMaxForAction(player);
        if (frame > visFrame)
            frame = visFrame;
        al::setVisAnimFrameForAction(player, frame);
    }
}

f32 PlayerAnimator::getMario3DWaitFrameMax() const {
    return al::getSklAnimFrameMax(mModelHolder->findModelActor("Normal"), "Wait");
}

f32 PlayerAnimator::getRunStartAnimFrameMax() const {
    return al::getSklAnimFrameMax(mModelHolder->getActor(), "RunStart");
}

f32 PlayerAnimator::getRunStartAnimBlendRate() const {
    return mRunStartAnimRate;
}

void PlayerAnimator::recordRunStartAnimRate(f32 rate) {
    mRunStartAnimRate = rate;
}

void PlayerAnimator::calcModelJointRootMtx(sead::Matrix34f* mtx) const {
    *mtx = *al::getJointMtxPtr(mModelHolder->getActor(), "JointRoot");
}

void PlayerAnimator::startPartsAnim(const sead::SafeString& animName) {
    al::LiveActor* player = mModelHolder->getActor();
    bool isUpperBodyAnim = _1a3;
    al::LiveActor* face = al::tryGetSubActor(player, "顔");
    const char* finalPart;
    if (isUpperBodyAnim) {
        if (face && !al::tryStartAction(face, animName.cstr()))
            al::startAction(face, "Wait");
        finalPart = "目";
    } else {
        if (face && !al::tryStartAction(face, animName.cstr()))
            al::startAction(face, "Wait");
        al::LiveActor* eye = al::tryGetSubActor(player, "目");
        if (eye && !al::tryStartAction(eye, animName.cstr()))
            al::startAction(eye, "Wait");
        al::LiveActor* head = al::tryGetSubActor(player, "頭");
        if (head && !al::tryStartAction(head, animName.cstr()))
            al::startAction(head, "Wait");
        al::LiveActor* leftHand = al::tryGetSubActor(player, "左手");
        if (leftHand && !al::tryStartAction(leftHand, animName.cstr()))
            al::startAction(leftHand, "Wait");
        finalPart = "右手";
    }
    al::LiveActor* actor = al::tryGetSubActor(player, finalPart);
    if (actor && !al::tryStartAction(actor, animName.cstr()))
        al::startAction(actor, "Wait");

    if (mIsNeedFullFaceAnim) {
        al::LiveActor* fullFace = al::tryGetSubActor(player, "顔");
        if (fullFace) {
            al::StringTmp<128> fullFaceAnim("%sFullFace", animName.cstr());
            al::tryStartAction(fullFace, fullFaceAnim.cstr());
        }
    }

    al::LiveActor* deco = mPlayerDeco;
    if (deco && !al::tryStartAction(deco, animName.cstr()))
        al::startAction(deco, "Wait");
}

void PlayerAnimator::setPartsAnimRate(f32 rate, const char* actionName) {
    al::LiveActor* player = mModelHolder->getActor();
    bool isUpperBodyAnim = _1a3;
    al::LiveActor* face = al::tryGetSubActor(player, "顔");
    const char* finalPart;
    if (isUpperBodyAnim) {
        if (face)
            setPartsAnimRateLocal(face, actionName, rate);
        finalPart = "目";
    } else {
        if (face)
            setPartsAnimRateLocal(face, actionName, rate);
        al::LiveActor* eye = al::tryGetSubActor(player, "目");
        if (eye)
            setPartsAnimRateLocal(eye, actionName, rate);
        al::LiveActor* head = al::tryGetSubActor(player, "頭");
        if (head)
            setPartsAnimRateLocal(head, actionName, rate);
        al::LiveActor* leftHand = al::tryGetSubActor(player, "左手");
        if (leftHand)
            setPartsAnimRateLocal(leftHand, actionName, rate);
        finalPart = "右手";
    }
    al::LiveActor* actor = al::tryGetSubActor(player, finalPart);
    if (actor)
        setPartsAnimRateLocal(actor, actionName, rate);
    if (mPlayerDeco)
        setPartsAnimRateLocal(mPlayerDeco, actionName, rate);
}

void PlayerAnimator::setPartsAnimFrame(f32 frame, const char* actionName) {
    al::LiveActor* player = mModelHolder->getActor();
    bool isUpperBodyAnim = _1a3;
    al::LiveActor* face = al::tryGetSubActor(player, "顔");
    const char* finalPart;
    if (isUpperBodyAnim) {
        if (face)
            setPartsAnimFrameLocal(face, actionName, frame);
        finalPart = "目";
    } else {
        if (face)
            setPartsAnimFrameLocal(face, actionName, frame);
        al::LiveActor* eye = al::tryGetSubActor(player, "目");
        if (eye)
            setPartsAnimFrameLocal(eye, actionName, frame);
        al::LiveActor* head = al::tryGetSubActor(player, "頭");
        if (head)
            setPartsAnimFrameLocal(head, actionName, frame);
        al::LiveActor* leftHand = al::tryGetSubActor(player, "左手");
        if (leftHand)
            setPartsAnimFrameLocal(leftHand, actionName, frame);
        finalPart = "右手";
    }
    al::LiveActor* actor = al::tryGetSubActor(player, finalPart);
    if (actor)
        setPartsAnimFrameLocal(actor, actionName, frame);
    if (mPlayerDeco)
        setPartsAnimFrameLocal(mPlayerDeco, actionName, frame);
}

void PlayerAnimator::copyAnimLocal() {
    al::LiveActor* target = mModelHolder->getActor();
    bool isJump = al::isStartWithString(mCurAnim.cstr(), "Jump");
    const char* fallback = isJump ? "Jump" : "Wait";

    if (mIsSubAnimPlaying) {
        if (al::isExistAction(target, mCurSubAnim.cstr())) {
            const char* subAnimName = mCurSubAnim.cstr();
            startAnimCommon(subAnimName);
            setAnimRateCommon(al::getActionFrameRate(mPlayer));
            setAnimFrameCommon(al::getActionFrame(mPlayer));
            if (!al::isExistAction(target, mCurAnim.cstr())) {
                mCurAnim = fallback;
                mAnimFrameCtrl->startAction(target, mCurAnim);
            }
        } else {
            mIsSubAnimPlaying = false;
            if (al::isExistAction(target, mCurAnim.cstr())) {
                mAnimFrameCtrl->startAction(target, mCurAnim);
                startAnimCommon(mCurAnim);
                setAnimRateCommon(al::getActionFrameRate(mPlayer));
                setAnimFrameCommon(al::getActionFrame(mPlayer));
            } else {
                mCurAnim = fallback;
                mAnimFrameCtrl->startAction(target, mCurAnim);
                const char* currentAnimName = mCurAnim.cstr();
                startAnimCommon(currentAnimName);
                setAnimRateCommon(1.0f);
            }
        }
    } else {
        if (al::isExistAction(target, mCurAnim.cstr())) {
            mAnimFrameCtrl->startAction(target, mCurAnim);
            startAnimCommon(mCurAnim);
            setAnimRateCommon(al::getActionFrameRate(mPlayer));
            setAnimFrameCommon(al::getActionFrame(mPlayer));
        } else {
            mCurAnim = fallback;
            mAnimFrameCtrl->startAction(target, mCurAnim);
            const char* currentAnimName = mCurAnim.cstr();
            startAnimCommon(currentAnimName);
            setAnimRateCommon(1.0f);
        }

        if (isJump) {
            setAnimFrame(al::getActionFrameMax(target));
            if (target->getEffectKeeper())
                al::tryDeleteEffect(target, "Jump");
        }
    }
    al::clearSklAnimInterpole(target);
}
