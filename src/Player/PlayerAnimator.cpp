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
void setActionFrame(al::LiveActor*, const char*, f32);
void setActionFrameRate(al::LiveActor*, const char*, f32);
void resetEyeControlAnim(al::LiveActor*);

const char* const sModelParts[] = {"顔", "目", "頭", "左手", "右手"};
}  // namespace

PlayerAnimator::PlayerAnimator(const PlayerModelHolder* modelHolder,
                               al::ActorDitherAnimator* ditherAnimator)
    : mModelHolder(modelHolder), mPlayerDeco(nullptr), mPlayer(nullptr),
      mAnimFrameCtrl(new PlayerAnimFrameCtrl), mCurAnim(""), mCurSubAnim(""),
      mCurUpperBodyAnim(""), _128(""), mDitherAnim(ditherAnimator),
      mEndEyeControlAnimDelay(0), mRunStartAnimRate(0.0f), mModelAlphaDelay(0),
      mIsNeedFullFaceAnim(false), mIsSubAnimPlaying(false), _1a3(false),
      mIsUpperBodyAnimHeadVisKeep(false), _1a5(false), _1a6(false), mIsSubAnimOnlyAir(false) {
    mSklAnimBlendWeights = new f32[6];
    mPlayer = modelHolder->getCurrentModelActor();
    mPlayerDeco = al::tryGetSubActor(modelHolder->findModelActor("Normal"), "");
    const sead::SafeString waitAnim("Wait");
    startAnim(waitAnim);
}

void PlayerAnimator::startAnim(const sead::SafeString& name) {
    const PlayerModelHolder* modelHolder = mModelHolder;
    al::LiveActor* player = modelHolder->getCurrentModelActor();
    sead::FixedSafeString<64>* curAnim = &mCurAnim;
    al::LiveActor* normalModel = modelHolder->findModelActor("Normal");
    curAnim->format("%s%s", name.cstr(), modelHolder->getModelSuffix().cstr());
    if (!al::isExistAction(normalModel, curAnim->cstr()))
        curAnim->format("%s", name.cstr());

    mAnimFrameCtrl->startAction(player, *curAnim);
    _1a1 = false;
    if (!mIsSubAnimPlaying) {
        al::startAction(mModelHolder->getCurrentModelActor(), curAnim->cstr());
        const sead::SafeString partsAnim(curAnim->cstr());
        startPartsAnim(partsAnim);
    }

    mAnimFrameCtrl->setRate(1.0f);
    if (!mIsSubAnimPlaying)
        setAnimRateCommon(1.0f);
}

void PlayerAnimator::updateAnimFrame() {
    if (mIsSubAnimPlaying)
        mAnimFrameCtrl->update();
    else
        mAnimFrameCtrl->updateSync(mModelHolder->getCurrentModelActor());
}

void PlayerAnimator::updateModel() {
    al::LiveActor* currentModel = mModelHolder->getCurrentModelActor();
    if (mPlayer == currentModel)
        return;

    copyAnimLocal();
    if (_1a3) {
        al::clearPartialSklAnim(mPlayer, 0);
        _1a3 = false;
    }
    mDitherAnim->reset();
    setModelAlpha(1.0f);
    mPlayer = currentModel;
}

void PlayerAnimator::copyAnim() {
    copyAnimLocal();
    if (_1a3) {
        al::clearPartialSklAnim(mPlayer, 0);
        _1a3 = false;
    }
    mDitherAnim->reset();
    setModelAlpha(1.0f);
}

void PlayerAnimator::startAnimCommon(const sead::SafeString& name) {
    al::startAction(mModelHolder->getCurrentModelActor(), name.cstr());
    const sead::SafeString partsAnim(name.cstr());
    startPartsAnim(partsAnim);
}

void PlayerAnimator::setAnimRate(f32 rate) {
    mAnimFrameCtrl->setRate(rate);
    if (!mIsSubAnimPlaying)
        setAnimRateCommon(rate);
}

void PlayerAnimator::startAnimSpinAttack(const sead::SafeString& name) {
    const PlayerModelHolder* modelHolder = mModelHolder;
    sead::FixedSafeString<64>* spinAnim = &_128;
    al::LiveActor* normalModel = modelHolder->findModelActor("Normal");
    spinAnim->format("%s%s", name.cstr(), modelHolder->getModelSuffix().cstr());
    if (!al::isExistAction(normalModel, spinAnim->cstr()))
        spinAnim->format("%s", name.cstr());
    startAnim(name);
}

void PlayerAnimator::setAnimRateCommon(f32 rate) {
    if (al::isSklAnimPlaying(mModelHolder->getCurrentModelActor(), 0))
        al::setSklAnimBlendFrameRateAll(mModelHolder->getCurrentModelActor(), rate, true);
    if (al::isVisAnimPlayingForAction(mModelHolder->getCurrentModelActor()))
        al::setVisAnimFrameRateForAction(mModelHolder->getCurrentModelActor(), rate);
    setPartsAnimRate(rate, mCurAnim.cstr());
}

void PlayerAnimator::setAnimFrame(f32 frame) {
    mAnimFrameCtrl->setFrame(frame);
    if (!mIsSubAnimPlaying)
        setAnimFrameCommon(frame);
}

void PlayerAnimator::setAnimFrameCommon(f32 frame) {
    if (al::isSklAnimExist(mModelHolder->getCurrentModelActor(), mCurAnim.cstr())) {
        al::setSklAnimBlendFrameAll(mModelHolder->getCurrentModelActor(), frame, true);
    } else if (al::isVisAnimExist(mModelHolder->getCurrentModelActor(), mCurAnim.cstr())) {
        f32 frameMax = al::getVisAnimFrameMaxForAction(mModelHolder->getCurrentModelActor());
        if (frameMax > frame)
            frameMax = frame;
        al::setVisAnimFrameForAction(mModelHolder->getCurrentModelActor(), frameMax);
    }
    setPartsAnimFrame(frame, mCurAnim.cstr());
}

bool PlayerAnimator::isAnimEnd() const {
    return mAnimFrameCtrl->isActionEnd();
}

bool PlayerAnimator::isAnim(const sead::SafeString& name) const {
    if (al::isEqualString(name.cstr(), mCurAnim.cstr()))
        return true;

    return al::isEqualString(name.cstr(), mAnimFrameCtrl->getActionName());
}

bool PlayerAnimator::isCurrentAnimOneTime() const {
    return mAnimFrameCtrl->isActionOneTime();
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
        al::clearSklAnimInterpole(mModelHolder->getCurrentModelActor());
}

void PlayerAnimator::startSubAnim(const sead::SafeString& name) {
    const PlayerModelHolder* modelHolder = mModelHolder;
    mIsSubAnimPlaying = true;
    mIsSubAnimOnlyAir = false;
    sead::FixedSafeString<64>* curSubAnim = &mCurSubAnim;
    al::LiveActor* normalModel = modelHolder->findModelActor("Normal");
    curSubAnim->format("%s%s", name.cstr(), modelHolder->getModelSuffix().cstr());
    if (!al::isExistAction(normalModel, curSubAnim->cstr()))
        curSubAnim->format("%s", name.cstr());

    al::startAction(mModelHolder->getCurrentModelActor(), curSubAnim->cstr());
    const sead::SafeString partsAnim(curSubAnim->cstr());
    startPartsAnim(partsAnim);
}

void PlayerAnimator::startSubAnimOnlyAir(const sead::SafeString& name) {
    startSubAnim(name);
    mIsSubAnimOnlyAir = true;
}

void PlayerAnimator::endSubAnim() {
    mIsSubAnimPlaying = false;
    mIsSubAnimOnlyAir = false;
    if (al::isEqualString(mCurAnim.cstr(), mCurSubAnim.cstr()))
        return;

    const char* actionName = mAnimFrameCtrl->getActionName();
    al::startAction(mModelHolder->getCurrentModelActor(), actionName);
    const sead::SafeString partsAnim(actionName);
    startPartsAnim(partsAnim);
    setAnimRate(mAnimFrameCtrl->getRate());
    setAnimFrame(mAnimFrameCtrl->getCurrentFrame());
    applyBlendWeight();
}

void PlayerAnimator::applyBlendWeight() {
    if (!_1a1)
        return;

    al::LiveActor* player = mModelHolder->getCurrentModelActor();
    f32* weights = mSklAnimBlendWeights;
    al::setSklAnimBlendWeightSixfold(player, weights[0], weights[1], weights[2], weights[3],
                                     weights[4], weights[5]);
    const char* actionName = al::getActionName(player);

    if (_1a3) {
        al::LiveActor* face = al::tryGetSubActor(player, "顔");
        if (face && al::isActionPlaying(face, actionName) && al::isSklAnimExist(face)) {
            f32* faceWeights = mSklAnimBlendWeights;
            al::setSklAnimBlendWeightSixfold(face, faceWeights[0], faceWeights[1], faceWeights[2],
                                             faceWeights[3], faceWeights[4], faceWeights[5]);
        }

        al::LiveActor* eye = al::tryGetSubActor(player, "目");
        if (eye && al::isActionPlaying(eye, actionName) && al::isSklAnimExist(eye)) {
            f32* eyeWeights = mSklAnimBlendWeights;
            al::setSklAnimBlendWeightSixfold(eye, eyeWeights[0], eyeWeights[1], eyeWeights[2],
                                             eyeWeights[3], eyeWeights[4], eyeWeights[5]);
        }
        return;
    }

    for (s32 i = 0; i < 5; i++) {
        al::LiveActor* part = al::tryGetSubActor(player, sModelParts[i]);
        if (part && al::isActionPlaying(part, actionName) && al::isSklAnimExist(part)) {
            f32* partWeights = mSklAnimBlendWeights;
            al::setSklAnimBlendWeightSixfold(part, partWeights[0], partWeights[1], partWeights[2],
                                             partWeights[3], partWeights[4], partWeights[5]);
        }
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

    al::LiveActor* actor = mModelHolder->getCurrentModelActor();
    if (al::isSklAnimExist(actor, al::getActionName(actor)))
        return al::isActionEnd(actor);

    return al::isVisAnimEnd(actor);
}

bool PlayerAnimator::isSubAnim(const sead::SafeString& name) const {
    if (!mIsSubAnimPlaying)
        return false;

    return al::isEqualString(name.cstr(), mCurSubAnim.cstr());
}

f32 PlayerAnimator::getSubAnimFrame() const {
    return al::getActionFrame(mModelHolder->getCurrentModelActor());
}

f32 PlayerAnimator::getSubAnimFrameMax() const {
    return al::getActionFrameMax(mModelHolder->getCurrentModelActor());
}

bool PlayerAnimator::isUpperBodyAnimAttached() const {
    return al::isPartialSklAnimAttached(mModelHolder->getCurrentModelActor(), 0);
}

bool PlayerAnimator::isUpperBodyAnimEnd() const {
    return al::isPartialSklAnimEnd(mModelHolder->getCurrentModelActor(), 0);
}

bool PlayerAnimator::isUpperBodyAnim(const sead::SafeString& name) const {
    return al::isPartialSklAnimPlaying(mModelHolder->getCurrentModelActor(), name.cstr(), 0);
}

void PlayerAnimator::startUpperBodyAnim(const sead::SafeString& name) {
    const PlayerModelHolder* modelHolder = mModelHolder;
    al::LiveActor* player = modelHolder->getCurrentModelActor();
    sead::FixedSafeString<64>* upperBodyAnim = &mCurUpperBodyAnim;
    al::LiveActor* normalModel = modelHolder->findModelActor("Normal");
    upperBodyAnim->format("%s%s", name.cstr(), modelHolder->getModelSuffix().cstr());
    if (!al::isExistAction(normalModel, upperBodyAnim->cstr()))
        upperBodyAnim->format("%s", name.cstr());

    al::startPartialSklAnim(player, upperBodyAnim->cstr(), 0, 0, nullptr);
    const sead::SafeString partsAnim(upperBodyAnim->cstr());
    startPartsPartialAnim(partsAnim);
    _1a3 = true;
    mIsUpperBodyAnimHeadVisKeep = false;
}

void PlayerAnimator::startPartsPartialAnim(const sead::SafeString& name) {
    al::LiveActor* player = mModelHolder->getCurrentModelActor();
    al::LiveActor* head = al::tryGetSubActor(player, "頭");
    if (head)
        al::tryStartAction(head, name.cstr());

    al::LiveActor* leftHand = al::tryGetSubActor(player, "左手");
    if (leftHand)
        al::tryStartAction(leftHand, name.cstr());

    al::LiveActor* rightHand = al::tryGetSubActor(player, "右手");
    if (rightHand)
        al::tryStartAction(rightHand, name.cstr());
}

void PlayerAnimator::startUpperBodyAnimSubParts(const sead::SafeString& name) {
    const PlayerModelHolder* modelHolder = mModelHolder;
    al::LiveActor* player = modelHolder->getCurrentModelActor();
    sead::FixedSafeString<64>* upperBodyAnim = &mCurUpperBodyAnim;
    al::LiveActor* normalModel = modelHolder->findModelActor("Normal");
    upperBodyAnim->format("%s%s", name.cstr(), modelHolder->getModelSuffix().cstr());
    if (!al::isExistAction(normalModel, upperBodyAnim->cstr()))
        upperBodyAnim->format("%s", name.cstr());

    al::startPartialSklAnim(player, upperBodyAnim->cstr(), 0, 1, nullptr);
    const sead::SafeString partsAnim(upperBodyAnim->cstr());
    startPartsPartialAnim(partsAnim);
    _1a3 = true;
    mIsUpperBodyAnimHeadVisKeep = false;
}

void PlayerAnimator::startUpperBodyAnimAndHeadVisKeep(const sead::SafeString& name) {
    startUpperBodyAnim(name);
    mIsUpperBodyAnimHeadVisKeep = true;
}

void PlayerAnimator::clearUpperBodyAnim() {
    al::clearPartialSklAnimWithInterpolate(mModelHolder->getCurrentModelActor(), 0, 5);
    al::LiveActor* player = mModelHolder->getCurrentModelActor();
    const char* actionName = al::getActionName(player);
    f32 actionFrame = al::getActionFrame(player);
    f32 actionRate = al::getActionFrameRate(player);
    bool isSpinAttack = al::isEqualString(actionName, _128.cstr());

    al::LiveActor* head = al::tryGetSubActor(player, "頭");
    if (isSpinAttack) {
        if (head && !al::isEqualString("頭", "頭")) {
            if (!al::tryStartAction(head, actionName))
                al::startAction(head, "Wait");
            setActionFrame(head, actionName, actionFrame);
            setActionFrameRate(head, actionName, actionRate);
        }

        al::LiveActor* leftHand = al::tryGetSubActor(player, "左手");
        if (leftHand && !al::isEqualString("頭", "左手")) {
            if (!al::tryStartAction(leftHand, actionName))
                al::startAction(leftHand, "Wait");
            setActionFrame(leftHand, actionName, actionFrame);
            setActionFrameRate(leftHand, actionName, actionRate);
        }

        al::LiveActor* rightHand = al::tryGetSubActor(player, "右手");
        if (rightHand && !al::isEqualString("頭", "右手")) {
            if (!al::tryStartAction(rightHand, actionName))
                al::startAction(rightHand, "Wait");
            setActionFrame(rightHand, actionName, actionFrame);
            setActionFrameRate(rightHand, actionName, actionRate);
        }
    } else {
        if (head) {
            if (!al::tryStartAction(head, actionName))
                al::startAction(head, "Wait");
            setActionFrame(head, actionName, actionFrame);
            setActionFrameRate(head, actionName, actionRate);
        }

        al::LiveActor* leftHand = al::tryGetSubActor(player, "左手");
        if (leftHand) {
            if (!al::tryStartAction(leftHand, actionName))
                al::startAction(leftHand, "Wait");
            setActionFrame(leftHand, actionName, actionFrame);
            setActionFrameRate(leftHand, actionName, actionRate);
        }

        al::LiveActor* rightHand = al::tryGetSubActor(player, "右手");
        if (rightHand) {
            if (!al::tryStartAction(rightHand, actionName))
                al::startAction(rightHand, "Wait");
            setActionFrame(rightHand, actionName, actionFrame);
            setActionFrameRate(rightHand, actionName, actionRate);
        }
    }

    if (mIsUpperBodyAnimHeadVisKeep) {
        al::LiveActor* headVis = al::tryGetSubActor(player, "頭");
        if (headVis) {
            al::startVisAnimForAction(headVis, mCurUpperBodyAnim.cstr());
            al::setVisAnimFrameForAction(headVis, al::getVisAnimFrameMaxForAction(headVis));
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
    const char* deadAnims[] = {"Dead01", "Dead02"};
    if (al::getRandom(0, 20)) {
        const sead::SafeString anim(deadAnims[al::getRandom(0, 2)]);
        startAnim(anim);
    } else {
        const sead::SafeString anim("Dead04");
        startAnim(anim);
    }
}

void PlayerAnimator::startPress() {
    mAnimFrameCtrl->setRate(0.0f);
    if (!mIsSubAnimPlaying)
        setAnimRateCommon(0.0f);
    mModelHolder->getCurrentModelActor()->getActorActionKeeper()->tryStartActionNoAnim("Press");
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
    if (mModelAlphaDelay) {
        mModelAlphaDelay = al::converge(mModelAlphaDelay, 0, 1);
        mDitherAnim->reset();
        alpha = 1.0f;
    } else {
        mDitherAnim->update();
        alpha = mDitherAnim->getDitherAlpha();
    }
    setModelAlpha(alpha);
}

void PlayerAnimator::setModelAlpha(f32 alpha) {
    al::LiveActor* player = mModelHolder->getCurrentModelActor();
    al::setModelAlphaMask(player, alpha);

    al::LiveActor* face = al::tryGetSubActor(player, "顔");
    if (face)
        al::setModelAlphaMask(face, al::getModelAlphaMask(player));

    al::LiveActor* eye = al::tryGetSubActor(player, "目");
    if (eye)
        al::setModelAlphaMask(eye, al::getModelAlphaMask(player));

    al::LiveActor* head = al::tryGetSubActor(player, "頭");
    if (head)
        al::setModelAlphaMask(head, al::getModelAlphaMask(player));

    al::LiveActor* leftHand = al::tryGetSubActor(player, "左手");
    if (leftHand)
        al::setModelAlphaMask(leftHand, al::getModelAlphaMask(player));

    al::LiveActor* rightHand = al::tryGetSubActor(player, "右手");
    if (rightHand)
        al::setModelAlphaMask(rightHand, al::getModelAlphaMask(player));
}

void PlayerAnimator::resetModelAlpha() {
    mDitherAnim->reset();
    setModelAlpha(1.0f);
}

namespace {
void setActionFrame(al::LiveActor* actor, const char* actionName, f32 frame) {
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

void setActionFrameRate(al::LiveActor* actor, const char* actionName, f32 rate) {
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
}  // namespace

void PlayerAnimator::endDemoInvalidateModelAlpha() {
    mModelAlphaDelay = mModelAlphaDelay > 1 ? mModelAlphaDelay : 1;
}

void PlayerAnimator::startSnapShotMode() {
    mDitherAnim->setClippingJudgeDistanceParam("SnapShotMode");
}

void PlayerAnimator::endSnapShotMode() {
    mDitherAnim->resetClippingJudgeDistanceParam();
}

void PlayerAnimator::startEyeControlAnim(bool isStop) {
    _1a5 = true;
    _1a6 = isStop;
    mEndEyeControlAnimDelay = 0;
}

void PlayerAnimator::endEyeControlAnim(s32 delay) {
    _1a5 = false;
    _1a6 = false;
    mEndEyeControlAnimDelay = delay;
    if (delay == 0)
        resetEyeControlAnim(mModelHolder->getCurrentModelActor());
}

void PlayerAnimator::clearEndEyeControlAnimDelay() {
    if (mEndEyeControlAnimDelay >= 1) {
        mEndEyeControlAnimDelay = 0;
        resetEyeControlAnim(mModelHolder->getCurrentModelActor());
    }
}

void PlayerAnimator::updateEyeControlAnim() {
    if (_1a5) {
        f32 eyeFrame = mEyeControlFrame;
        al::LiveActor* eye = al::tryGetSubActor(mModelHolder->getCurrentModelActor(), "目");
        if (eye && al::isMtsAnimExist(eye, "EyeMove"))
            al::startMtsAnimAndSetFrameAndStop(eye, "EyeMove", eyeFrame);
    } else if (mEndEyeControlAnimDelay >= 1) {
        mEndEyeControlAnimDelay = al::converge(mEndEyeControlAnimDelay, 0, 1);
        if (mEndEyeControlAnimDelay == 0)
            resetEyeControlAnim(mModelHolder->getCurrentModelActor());
    }
}

void PlayerAnimator::startRightHandAnim(const char* animName) {
    al::LiveActor* rightHand = al::tryGetSubActor(mModelHolder->getCurrentModelActor(), "右手");
    if (rightHand)
        al::startAction(rightHand, animName);
}

void PlayerAnimator::overwrideYoshiEatVis() {
    al::tryStartVisAnimIfNotPlayingForAction(mModelHolder->getCurrentModelActor(), "Eat");
}

void PlayerAnimator::restartYoshiActionVis() {
    if (*mCurAnim.getStringTop() == sead::SafeString::cNullChar)
        return;

    al::LiveActor* player = mModelHolder->getCurrentModelActor();
    al::startVisAnimForAction(player, "Wait");
    if (!al::tryStartVisAnimIfExistForAction(player, mCurAnim.cstr()))
        return;

    f32 actionFrameMax = al::getActionFrameMax(player);
    f32 visFrameMax = al::getVisAnimFrameMaxForAction(player);
    visFrameMax = actionFrameMax > visFrameMax ? visFrameMax : actionFrameMax;
    al::setVisAnimFrameForAction(player, visFrameMax);
}

f32 PlayerAnimator::getMario3DWaitFrameMax() const {
    return al::getSklAnimFrameMax(mModelHolder->findModelActor("Normal"), "Wait");
}

f32 PlayerAnimator::getRunStartAnimFrameMax() const {
    return al::getSklAnimFrameMax(mModelHolder->getCurrentModelActor(), "RunStart");
}

f32 PlayerAnimator::getRunStartAnimBlendRate() const {
    return mRunStartAnimRate;
}

void PlayerAnimator::recordRunStartAnimRate(f32 rate) {
    mRunStartAnimRate = rate;
}

namespace {
void resetEyeControlAnim(al::LiveActor* player) {
    const char* actionName = al::getActionName(player);
    f32 actionFrame = al::getActionFrame(player);
    f32 actionRate = al::getActionFrameRate(player);
    al::LiveActor* eye = al::tryGetSubActor(player, "目");
    if (!eye)
        return;

    if (al::tryStartAction(eye, actionName)) {
        setActionFrameRate(eye, actionName, actionRate);
        setActionFrame(eye, actionName, actionFrame);
    } else {
        al::startAction(eye, "Wait");
    }

    if (al::isMtsAnimPlaying(eye, "EyeMove"))
        al::startMtsAnim(eye, "EyeReset");
}
}  // namespace

void PlayerAnimator::calcModelJointRootMtx(sead::Matrix34f* out) const {
    *out = *al::getJointMtxPtr(mModelHolder->getCurrentModelActor(), "JointRoot");
}

// NON_MATCHING: target/current are both 652 bytes, but the whole-function register assignment swaps this/name (X19/X20); next source-level hypothesis is shortening the player/face temporary lifetimes before finalPartName selection.
void PlayerAnimator::startPartsAnim(const sead::SafeString& name) {
    al::LiveActor* player = mModelHolder->getCurrentModelActor();
    bool isUpperBodyAnim = _1a3;
    al::LiveActor* face = al::tryGetSubActor(player, "顔");

    const char* finalPartName;
    if (isUpperBodyAnim) {
        if (face) {
            if (!al::tryStartAction(face, name.cstr()))
                al::startAction(face, "Wait");
        }
        finalPartName = "目";
    } else {
        if (face) {
            if (!al::tryStartAction(face, name.cstr()))
                al::startAction(face, "Wait");
        }
        al::LiveActor* eye = al::tryGetSubActor(player, "目");
        if (eye) {
            if (!al::tryStartAction(eye, name.cstr()))
                al::startAction(eye, "Wait");
        }

        al::LiveActor* head = al::tryGetSubActor(player, "頭");
        if (head) {
            if (!al::tryStartAction(head, name.cstr()))
                al::startAction(head, "Wait");
        }

        al::LiveActor* leftHand = al::tryGetSubActor(player, "左手");
        if (leftHand) {
            if (!al::tryStartAction(leftHand, name.cstr()))
                al::startAction(leftHand, "Wait");
        }
        finalPartName = "右手";
    }

    al::LiveActor* finalPart = al::tryGetSubActor(player, finalPartName);
    if (finalPart) {
        if (!al::tryStartAction(finalPart, name.cstr()))
            al::startAction(finalPart, "Wait");
    }

    if (mIsNeedFullFaceAnim) {
        al::LiveActor* fullFace = al::tryGetSubActor(player, "顔");
        if (fullFace) {
            al::StringTmp<128> fullFaceAnim("%sFullFace", name.cstr());
            al::tryStartAction(fullFace, fullFaceAnim.cstr());
        }
    }

    if (mPlayerDeco) {
        if (!al::tryStartAction(mPlayerDeco, name.cstr()))
            al::startAction(mPlayerDeco, "Wait");
    }
}

void PlayerAnimator::setPartsAnimRate(f32 rate, const char* actionName) {
    al::LiveActor* player = mModelHolder->getCurrentModelActor();
    bool isUpperBodyAnim = _1a3;
    al::LiveActor* face = al::tryGetSubActor(player, "顔");

    const char* finalPartName;
    if (isUpperBodyAnim) {
        if (face)
            setActionFrameRate(face, actionName, rate);
        finalPartName = "目";
    } else {
        if (face)
            setActionFrameRate(face, actionName, rate);
        al::LiveActor* eye = al::tryGetSubActor(player, "目");
        if (eye)
            setActionFrameRate(eye, actionName, rate);
        al::LiveActor* head = al::tryGetSubActor(player, "頭");
        if (head)
            setActionFrameRate(head, actionName, rate);
        al::LiveActor* leftHand = al::tryGetSubActor(player, "左手");
        if (leftHand)
            setActionFrameRate(leftHand, actionName, rate);
        finalPartName = "右手";
    }

    al::LiveActor* finalPart = al::tryGetSubActor(player, finalPartName);
    if (finalPart)
        setActionFrameRate(finalPart, actionName, rate);
    if (mPlayerDeco)
        setActionFrameRate(mPlayerDeco, actionName, rate);
}

void PlayerAnimator::setPartsAnimFrame(f32 frame, const char* actionName) {
    al::LiveActor* player = mModelHolder->getCurrentModelActor();
    bool isUpperBodyAnim = _1a3;
    al::LiveActor* face = al::tryGetSubActor(player, "顔");

    const char* finalPartName;
    if (isUpperBodyAnim) {
        if (face)
            setActionFrame(face, actionName, frame);
        finalPartName = "目";
    } else {
        if (face)
            setActionFrame(face, actionName, frame);
        al::LiveActor* eye = al::tryGetSubActor(player, "目");
        if (eye)
            setActionFrame(eye, actionName, frame);
        al::LiveActor* head = al::tryGetSubActor(player, "頭");
        if (head)
            setActionFrame(head, actionName, frame);
        al::LiveActor* leftHand = al::tryGetSubActor(player, "左手");
        if (leftHand)
            setActionFrame(leftHand, actionName, frame);
        finalPartName = "右手";
    }

    al::LiveActor* finalPart = al::tryGetSubActor(player, finalPartName);
    if (finalPart)
        setActionFrame(finalPart, actionName, frame);
    if (mPlayerDeco)
        setActionFrame(mPlayerDeco, actionName, frame);
}

void PlayerAnimator::copyAnimLocal() {
    al::LiveActor* model = mModelHolder->getCurrentModelActor();
    const bool isJumpAnim = al::isStartWithString(mCurAnim.cstr(), "Jump");
    const char* fallbackAnim = isJumpAnim ? "Jump" : "Wait";

    if (mIsSubAnimPlaying) {
        if (al::isExistAction(model, mCurSubAnim.cstr())) {
            const char* subAnim = mCurSubAnim.cstr();
            al::startAction(mModelHolder->getCurrentModelActor(), subAnim);
            sead::SafeString partsAnim(subAnim);
            startPartsAnim(partsAnim);
            setAnimRateCommon(al::getActionFrameRate(mPlayer));
            setAnimFrameCommon(al::getActionFrame(mPlayer));
            if (!al::isExistAction(model, mCurAnim.cstr())) {
                mCurAnim = fallbackAnim;
                mAnimFrameCtrl->startAction(model, mCurAnim);
            }
        } else {
            mIsSubAnimPlaying = false;
            if (al::isExistAction(model, mCurAnim.cstr())) {
                mAnimFrameCtrl->startAction(model, mCurAnim);
                al::startAction(mModelHolder->getCurrentModelActor(), mCurAnim.cstr());
                sead::SafeString partsAnim(mCurAnim.cstr());
                startPartsAnim(partsAnim);
                setAnimRateCommon(al::getActionFrameRate(mPlayer));
                setAnimFrameCommon(al::getActionFrame(mPlayer));
            } else {
                mCurAnim = fallbackAnim;
                mAnimFrameCtrl->startAction(model, mCurAnim);
                const char* currentAnim = mCurAnim.cstr();
                al::startAction(mModelHolder->getCurrentModelActor(), currentAnim);
                sead::SafeString partsAnim(currentAnim);
                startPartsAnim(partsAnim);
                setAnimRateCommon(1.0f);
            }
        }
    } else {
        if (al::isExistAction(model, mCurAnim.cstr())) {
            mAnimFrameCtrl->startAction(model, mCurAnim);
            al::startAction(mModelHolder->getCurrentModelActor(), mCurAnim.cstr());
            sead::SafeString partsAnim(mCurAnim.cstr());
            startPartsAnim(partsAnim);
            setAnimRateCommon(al::getActionFrameRate(mPlayer));
            setAnimFrameCommon(al::getActionFrame(mPlayer));
        } else {
            mCurAnim = fallbackAnim;
            mAnimFrameCtrl->startAction(model, mCurAnim);
            const char* currentAnim = mCurAnim.cstr();
            al::startAction(mModelHolder->getCurrentModelActor(), currentAnim);
            sead::SafeString partsAnim(currentAnim);
            startPartsAnim(partsAnim);
            setAnimRateCommon(1.0f);
        }

        if (isJumpAnim) {
            setAnimFrame(al::getActionFrameMax(model));
            if (model->getEffectKeeper())
                al::tryDeleteEffect(model, "Jump");
        }
    }
    al::clearSklAnimInterpole(model);
}
