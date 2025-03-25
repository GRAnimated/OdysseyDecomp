#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/HostIO/HioNode.h"

namespace al {
class ActorResource;
class AnimPlayerMat;
class AnimPlayerSimple;
class AnimPlayerSkl;
class AnimPlayerVis;
class ModelCtrl;
class DitherAnimator;

class ModelKeeper : public HioNode {
public:
    ModelKeeper();

    virtual ~ModelKeeper();

    void calc(const sead::Matrix34f&, const sead::Vector3f&);
    void initResource();
    void createMatAnimForProgram(s32);
    void setDitherAnimator(DitherAnimator* ditherAnimator);

    void update();
    void show();
    void hide();

    const char* getName() const { return mName; }

    ModelCtrl* getModelCtrl() const { return mModelCtrl; }

    AnimPlayerSkl* getAnimSkl() const { return mAnimSkl; }

    AnimPlayerMat* getAnimMtp() const { return mAnimMtp; }

    AnimPlayerMat* getAnimMts() const { return mAnimMts; }

    AnimPlayerMat* getAnimMcl() const { return mAnimMcl; }

    AnimPlayerMat* getAnimMat() const { return mAnimMat; }

    AnimPlayerVis* getAnimVis() const { return mAnimVis; }

    AnimPlayerVis* getAnimVisForAction() const { return mAnimVisForAction; }

    bool isFixedModel() const { return mIsFixedModel; }

    void setFixedModelFlag(bool isFixedModel) { mIsFixedModel = isFixedModel; }

    bool isIgnoreUpdateDrawClipping() const { return mIsIgnoreUpdateDrawClipping; }

    void setIgnoreUpdateDrawClipping(bool isIgnoreUpdateDrawClipping) {
        mIsIgnoreUpdateDrawClipping = isIgnoreUpdateDrawClipping;
    }

    bool isNeedSetBaseMtxAndCalcAnim() const { return mIsNeedSetBaseMtxAndCalcAnim; }

    void setNeedSetBaseMtxAndCalcAnimFlag(bool isNeedSetBaseMtxAndCalcAnim) {
        mIsNeedSetBaseMtxAndCalcAnim = isNeedSetBaseMtxAndCalcAnim;
    }

private:
    const char* mName;
    ModelCtrl* mModelCtrl;
    ActorResource* mActorRes;
    AnimPlayerSkl* mAnimSkl;
    AnimPlayerMat* mAnimMtp;
    AnimPlayerMat* mAnimMts;
    AnimPlayerMat* mAnimMcl;
    AnimPlayerMat* mAnimMat;
    AnimPlayerVis* mAnimVisForAction;
    AnimPlayerVis* mAnimVis;
    void* _58;
    bool _60;
    bool mIsFixedModel;
    bool mIsIgnoreUpdateDrawClipping;
    bool mIsNeedSetBaseMtxAndCalcAnim;
};

}  // namespace al
