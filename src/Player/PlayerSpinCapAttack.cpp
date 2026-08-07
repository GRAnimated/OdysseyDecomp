#include "Player/PlayerSpinCapAttack.h"

#include "Library/Math/MathUtil.h"

#include "Player/HackCap.h"
#include "Player/PlayerAnimator.h"
#include "Player/PlayerCapFunction.h"
#include "Player/PlayerConst.h"
#include "Player/PlayerCounterAfterCapCatch.h"
#include "Player/PlayerInput.h"
#include "Player/PlayerInputFunction.h"
#include "Player/PlayerJudgePreInputCapThrow.h"
#include "Player/PlayerTrigger.h"

namespace {
// NON_MATCHING: the anonymous 528-byte target has no metadata label for tools/check; local size and
// 132-opcode sequence match the corpus exactly.
void makeSpinCapAnimName(sead::BufferedSafeString* animName, const char* baseName,
                         const PlayerInput* input, bool isSpinAttack,
                         const sead::Vector3f& throwStartDir, const sead::Vector2f& doubleThrowDir,
                         const sead::Vector2f& throwDir, s32 attackFrame) {
    if (input->isThrowTypeRolling(doubleThrowDir)) {
        if (!(doubleThrowDir.y > 0.0f))
            animName->format("%s%s", baseName, "DoubleDown");
        else
            animName->format("%s%s", baseName, "DoubleUp");
        return;
    }
    if (input->isThrowTypeSpiral(doubleThrowDir)) {
        if (!(doubleThrowDir.x > 0.0f))
            animName->format("%s%s", baseName, "DoubleLeft");
        else
            animName->format("%s%s", baseName, "DoubleRight");
        return;
    }
    if (!al::isNearZero(throwStartDir, 0.001f)) {
        if (!(throwStartDir.y < 0.0f))
            animName->format("%s%s", baseName, "Left");
        else
            animName->format("%s%s", baseName, "Right");
        return;
    }
    if (attackFrame == 1 && !isSpinAttack) {
        if (!input->isThrowTypeLeftRight(throwDir))
            animName->format("%s", baseName);
        else if (throwDir.x > 0.0f)
            animName->format("%s%s", baseName, "Right");
        else
            animName->format("%s%s", baseName, "Left");
        return;
    }

    const s32 type = isSpinAttack ? 3 : (attackFrame == 2 ? 2 : 3);
    if (!input->isThrowTypeLeftRight(throwDir))
        animName->format("%s%dRight", "SpinCapStart", type);
    else if (!(throwDir.x > 0.0f))
        animName->format("%s%d%s", "SpinCapStart", type, "Left");
    else
        animName->format("%s%d%s", "SpinCapStart", type, "Right");
}
}  // namespace

PlayerSpinCapAttack::PlayerSpinCapAttack(HackCap* hackCap, const PlayerConst* playerConst,
                                         const PlayerTrigger* trigger, const PlayerInput* input,
                                         const PlayerCounterAfterCapCatch* counterAfterCapCatch,
                                         const PlayerJudgePreInputCapThrow* judgePreInputCapThrow)
    : mHackCap(hackCap), mConst(playerConst), mTrigger(trigger), mInput(input),
      mCounterAfterCapCatch(counterAfterCapCatch), mJudgePreInputCapThrow(judgePreInputCapThrow),
      mIsCooperate(false), mIsSpinAttack(false), mThrowStartDir(0.0f, 0.0f, 0.0f), mAttackFrame(0),
      mSwingFlags(0), mThrowDir(0.0f, 0.0f), mDoubleThrowDir(0.0f, 0.0f), mThrowAngle(0.0f),
      mSpinAnimName(""), mSpinMissAnimName("") {}

void PlayerSpinCapAttack::clearAttackInfo() {
    mIsSpinAttack = false;
    mAttackFrame = 0;
}

void PlayerSpinCapAttack::setupAttackInfo() {
    if (mAttackFrame != 0 && mCounterAfterCapCatch->isCapCatch())
        mAttackFrame++;
    else
        mAttackFrame = 1;

    if (mCounterAfterCapCatch->isCapCatch() && mAttackFrame >= 3) {
        mIsSpinAttack = true;
        mAttackFrame = 0;
    } else {
        mIsSpinAttack = false;
    }

    mIsCooperate = mJudgePreInputCapThrow->isRecordedCooperate();
    mSwingFlags = 0;
    mThrowStartDir.set(0.0f, 0.0f, 0.0f);
    mThrowDir.set(0.0f, 0.0f);
    mDoubleThrowDir.set(0.0f, 0.0f);
    mThrowAngle = 0.0f;

    if (mJudgePreInputCapThrow->getRecordedThrowType() == 4) {
        mSwingFlags = 0x101;
        mDoubleThrowDir = mJudgePreInputCapThrow->getRecordedDoubleThrowDir();
    } else if (mTrigger->isOnSpinMoveCapThrow()) {
        mSwingFlags = 0x101;
        if (!mTrigger->isOn(static_cast<PlayerTrigger::EActionTrigger>(27)))
            mDoubleThrowDir = sead::Vector2f::ex;
        else
            mDoubleThrowDir.set(-sead::Vector2f::ex.x, -sead::Vector2f::ex.y);
    } else if (mTrigger->isOnHipDropCancelThrow()) {
        mSwingFlags = 0x101;
        mDoubleThrowDir.set(-sead::Vector2f::ey.x, -sead::Vector2f::ey.y);
    } else if (mJudgePreInputCapThrow->getRecordedThrowType() == 2) {
        isSwingLeft = true;
        mThrowDir = mJudgePreInputCapThrow->getRecordedThrowDir();
    } else if (mJudgePreInputCapThrow->getRecordedThrowType() == 3) {
        isSwingRight = true;
        mThrowDir = mJudgePreInputCapThrow->getRecordedThrowDir();
    }

    if (mInput->isThrowTypeRolling(mDoubleThrowDir) || mInput->isThrowTypeSpiral(mDoubleThrowDir)) {
        mIsSpinAttack = false;
        mThrowStartDir.set(0.0f, 0.0f, 0.0f);
    } else if (mInput->isThrowTypeLeftRight(mThrowDir)) {
        f32 angle;
        if (al::isNearZeroOrGreater(mThrowDir.x, 0.001f)) {
            angle = al::calcAngleDegree(sead::Vector2f::ex, mThrowDir);
        } else {
            sead::Vector2f left;
            left.set(-sead::Vector2f::ex.x, -sead::Vector2f::ex.y);
            angle = al::calcAngleDegree(left, mThrowDir);
        }
        const f32 absoluteAngle = angle > 0.0f ? angle : -angle;
        const f32 rate = al::calcRate01(absoluteAngle * 1.2f, 6.0f, 70.0f);
        if (rate > 0.0f)
            mThrowAngle = al::sign(angle) * al::lerpValue(6.0f, 70.0f, al::easeOut(rate));
    }
}

void PlayerSpinCapAttack::startCapSpinAttack(PlayerAnimator* animator, const PlayerInput*) {
    if (animator->isUpperBodyAnimAttached())
        animator->clearUpperBodyAnim();
    makeSpinCapAnimName(&mSpinAnimName, "SpinCapStart", mInput, mIsSpinAttack, mThrowStartDir,
                        mDoubleThrowDir, mThrowDir, mAttackFrame);
    animator->startAnimSpinAttack(sead::SafeString(mSpinAnimName.cstr()));
    animator->startSubAnim(sead::SafeString(mSpinAnimName.cstr()));
    mHackCap->startSpinAttack(mSpinAnimName.cstr());
    mSpinMissAnimName.format("");
}

void PlayerSpinCapAttack::startCapSpinAttackAir(PlayerAnimator* animator, const PlayerInput*) {
    if (animator->isUpperBodyAnimAttached())
        animator->clearUpperBodyAnim();
    makeSpinCapAnimName(&mSpinAnimName, "SpinCapAirStart", mInput, mIsSpinAttack, mThrowStartDir,
                        mDoubleThrowDir, mThrowDir, mAttackFrame);
    animator->startAnimSpinAttack(sead::SafeString(mSpinAnimName.cstr()));
    if (animator->isSubAnimPlaying())
        animator->endSubAnim();
    mHackCap->startSpinAttack(mSpinAnimName.cstr());
    mSpinMissAnimName.format("");
}

void PlayerSpinCapAttack::startCapSpinAttackSwim(PlayerAnimator* animator, const PlayerInput*) {
    mIsSpinAttack = false;
    if (animator->isUpperBodyAnimAttached())
        animator->clearUpperBodyAnim();
    makeSpinCapAnimName(&mSpinAnimName, "SwimSpinCapStart", mInput, mIsSpinAttack, mThrowStartDir,
                        mDoubleThrowDir, mThrowDir, 1);
    animator->startAnimSpinAttack(sead::SafeString(mSpinAnimName.cstr()));
    mHackCap->startSpinAttack(mSpinAnimName.cstr());
    mSpinMissAnimName.format("");
}

void PlayerSpinCapAttack::startCapThrow(const sead::Vector3f& startPos,
                                        const sead::Vector3f& throwDir, f32 power, bool isAppend,
                                        const sead::Vector3f& targetPos) {
    u64 swingHand = 2;
    if (static_cast<u8>(mSwingFlags))
        swingHand = 3;
    if (mSwingFlags < 0x100)
        swingHand = static_cast<u8>(mSwingFlags);

    mHackCap->startThrow(mIsSpinAttack, startPos, throwDir, power, mThrowDir, mDoubleThrowDir,
                         mThrowStartDir, isAppend, targetPos,
                         static_cast<HackCap::SwingHandType>(swingHand), mIsCooperate, mThrowAngle,
                         mAttackFrame);
}

void PlayerSpinCapAttack::attackSpinMsg(al::HitSensor* self, al::HitSensor* other) {
    mHackCap->attackSpin(self, other, mConst->getCollisionRadius());
}

bool PlayerSpinCapAttack::tryCancelCapState(PlayerAnimator* animator) {
    if (!mHackCap->cancelCapState())
        return false;
    (CapFunction::putOnCapPlayer)(mHackCap, animator);
    return true;
}

bool PlayerSpinCapAttack::tryStartCapSpinGroundMiss(PlayerAnimator* animator) {
    if (mSpinAnimName.getStringTop()[0] == sead::SafeString::cNullChar ||
        (animator->isSubAnimPlaying() && !animator->isSubAnim(mSpinAnimName)))
        return false;

    mSpinMissAnimName.format("SpinSeparate");
    animator->startSubAnim(sead::SafeString(mSpinMissAnimName.cstr()));
    return true;
}

bool PlayerSpinCapAttack::tryStartCapSpinAirMiss(PlayerAnimator* animator) {
    if (mSpinAnimName.getStringTop()[0] == sead::SafeString::cNullChar ||
        (animator->isSubAnimPlaying() && !animator->isSubAnim(mSpinAnimName)))
        return false;

    if (animator->isAnim(sead::SafeString(mSpinAnimName.cstr())))
        animator->startAnim("Fall");

    mSpinMissAnimName.format("SpinSeparate");
    animator->startSubAnimOnlyAir(sead::SafeString(mSpinMissAnimName.cstr()));
    return true;
}

void PlayerSpinCapAttack::startSpinSeparate(PlayerAnimator* animator) {
    animator->startAnim("SpinSeparate");
    animator->startSubAnim("SpinSeparate");
}

void PlayerSpinCapAttack::startSpinSeparateSwim(PlayerAnimator* animator) {
    animator->startAnim("SpinSeparateSwim");
    animator->startSubAnim("SpinSeparateSwim");
}

void PlayerSpinCapAttack::startSpinSeparateSwimSurface(PlayerAnimator* animator) {
    animator->startAnim("SwimSpinCapStartRight");
    animator->startSubAnim("SwimSpinCapStartRight");
}

bool PlayerSpinCapAttack::isCapSpinAttack() const {
    return mHackCap->isSpinAttack();
}

bool PlayerSpinCapAttack::isValidAttackSensor(const PlayerAnimator* animator) const {
    f32 frame;
    if (animator->isSubAnim(sead::SafeString(mSpinAnimName.cstr()))) {
        frame = animator->getSubAnimFrame();
    } else {
        if (!animator->isAnim(sead::SafeString(mSpinAnimName.cstr())))
            return false;
        frame = animator->getAnimFrame();
    }
    return !(frame > static_cast<f32>(mConst->getSpinAttackFrame()));
}

bool PlayerSpinCapAttack::isEnablePlaySpinCapMiss(const PlayerAnimator* animator) const {
    if (animator->isSubAnimPlaying())
        return !animator->isSubAnim(sead::SafeString(mSpinMissAnimName.cstr()));
    return true;
}

bool PlayerSpinCapAttack::isSeparateSingleSpin() const {
    return rs::isSeparatePlay(mHackCap) && !mIsCooperate;
}

bool PlayerSpinCapAttack::isThrowSwingRightDir() const {
    return mInput->isThrowTypeLeftRight(mThrowDir) && al::isNearZeroOrGreater(mThrowDir.x, 0.001f);
}

s32 PlayerSpinCapAttack::getThrowFrameGround() const {
    if (mIsSpinAttack)
        return mConst->getSpinCapThrowFrameContinuous();
    if (!al::isNearZero(mThrowStartDir, 0.001f) || isSwingLeft || isSwingRight)
        return mConst->getSpinCapThrowFrameSwing();
    if (mAttackFrame == 2)
        return mConst->getSpinCapThrowFrameSwing();
    return mConst->getSpinCapThrowFrame();
}

s32 PlayerSpinCapAttack::getThrowFrameAir() const {
    if (mIsSpinAttack)
        return mConst->getSpinCapThrowFrameContinuous();
    if (!al::isNearZero(mThrowStartDir, 0.001f) || isSwingLeft || isSwingRight)
        return mConst->getSpinCapThrowFrameSwing();
    if (mAttackFrame == 2)
        return mConst->getSpinCapThrowFrameSwing();
    return mConst->getSpinCapThrowFrameAir();
}

s32 PlayerSpinCapAttack::getThrowFrameSwim() const {
    if (isSwingLeft || isSwingRight)
        return mConst->getSpinCapThrowFrameSwing();
    return mConst->getSpinCapThrowFrameSwim();
}
