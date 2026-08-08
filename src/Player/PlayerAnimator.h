#pragma once

#include <math/seadMatrix.h>
#include <prim/seadSafeString.h>

namespace al {
class ActorDitherAnimator;
class LiveActor;
}  // namespace al

class PlayerModelHolder;
class PlayerAnimFrameCtrl;

class PlayerAnimator {
public:
    PlayerAnimator(const PlayerModelHolder* modelHolder, al::ActorDitherAnimator* ditherAnimator);
    void startAnim(const sead::SafeString& name);
    void updateAnimFrame();
    void updateModel();
    void copyAnim();
    void startAnimCommon(const sead::SafeString& name);
    void setAnimRate(f32 rate);
    void startAnimSpinAttack(const sead::SafeString& name);
    void setAnimRateCommon(f32 rate);
    void setAnimFrame(f32 frame);
    void setAnimFrameCommon(f32 frame);
    bool isAnimEnd() const;
    bool isAnim(const sead::SafeString& name) const;
    bool isCurrentAnimOneTime() const;
    f32 getAnimFrame() const;
    f32 getAnimFrameMax() const;
    f32 getAnimFrameRate() const;
    void clearInterpolation();
    void startSubAnim(const sead::SafeString& name);
    void startSubAnimOnlyAir(const sead::SafeString& name);
    void endSubAnim();
    void applyBlendWeight();
    void setSubAnimFrame(f32 frame);
    void setSubAnimRate(f32 rate);
    bool isSubAnimEnd() const;
    bool isSubAnim(const sead::SafeString& name) const;
    f32 getSubAnimFrame() const;
    f32 getSubAnimFrameMax() const;
    bool isUpperBodyAnimAttached() const;
    bool isUpperBodyAnimEnd() const;
    bool isUpperBodyAnim(const sead::SafeString& name) const;
    void startUpperBodyAnim(const sead::SafeString& name);
    void startPartsPartialAnim(const sead::SafeString& name);
    void startUpperBodyAnimSubParts(const sead::SafeString& name);
    void startUpperBodyAnimAndHeadVisKeep(const sead::SafeString& name);
    void clearUpperBodyAnim();
    void setBlendWeight(f32 weight0, f32 weight1, f32 weight2, f32 weight3, f32 weight4, f32 weight5);
    f32 getBlendWeight(s32 index);
    void startAnimDead();
    void validateFullFaceAnim() { mIsNeedFullFaceAnim = true; }
    void startPress();
    void forceCapOn();
    void forceCapOff();
    f32 getModelAlpha() const;
    void updateModelAlpha();
    void setModelAlpha(f32 alpha);
    void resetModelAlpha();
    void endDemoInvalidateModelAlpha();
    void startSnapShotMode();
    void endSnapShotMode();
    void startEyeControlAnim(bool isStop);
    void endEyeControlAnim(s32 delay);
    void clearEndEyeControlAnimDelay();
    void updateEyeControlAnim();
    void startRightHandAnim(const char* animName);
    void overwrideYoshiEatVis();
    void restartYoshiActionVis();
    f32 getMario3DWaitFrameMax() const;
    f32 getRunStartAnimFrameMax() const;
    f32 getRunStartAnimBlendRate() const;
    void recordRunStartAnimRate(f32 rate);
    void calcModelJointRootMtx(sead::Matrix34f* out) const;
    void startPartsAnim(const sead::SafeString& name);
    void setPartsAnimRate(f32 rate, const char* actionName);
    void setPartsAnimFrame(f32 frame, const char* actionName);
    void copyAnimLocal();

    bool isSubAnimPlaying() const { return mIsSubAnimPlaying; }
    bool isSubAnimOnlyAir() const { return mIsSubAnimOnlyAir; }
    bool isEyeControlAnimActive() const { return _1a5; }
    bool isEyeControlAnimTilt() const { return _1a6; }
    void setEyeControlFrame(f32 frame) { mEyeControlFrame = frame; }

private:
    const PlayerModelHolder* mModelHolder;
    al::LiveActor* mPlayerDeco;
    al::LiveActor* mPlayer;
    PlayerAnimFrameCtrl* mAnimFrameCtrl;
    sead::FixedSafeString<64> mCurAnim;
    sead::FixedSafeString<64> mCurSubAnim;
    sead::FixedSafeString<64> mCurUpperBodyAnim;
    sead::FixedSafeString<64> _128;
    al::ActorDitherAnimator* mDitherAnim;
    f32* mSklAnimBlendWeights;
    f32 mEyeControlFrame = 0.0f;
    s32 mEndEyeControlAnimDelay;
    f32 mRunStartAnimRate;
    s32 mModelAlphaDelay;
    bool mIsNeedFullFaceAnim = false;
    bool _1a1;
    bool mIsSubAnimPlaying;
    bool _1a3;
    bool mIsUpperBodyAnimHeadVisKeep;
    bool _1a5;
    bool _1a6;
    bool mIsSubAnimOnlyAir;
};

static_assert(sizeof(PlayerAnimator) == 0x1a8);
