#include "Player/PlayerModelChanger2D3D.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"

#include "Player/PlayerInput.h"
#include "Player/PlayerModelHolder.h"

namespace {
const char* sModelNames2D[] = {"Mini2D", "Normal2D", "Flower2D"};
const char* sModelNames3D[] = {"Mini", "Normal", "Flower"};

const char* getModelName(s32 model, bool is2D) {
    if (is2D) {
        if ((u32)model < 3)
            return sModelNames2D[model];
    } else if ((u32)model < 3) {
        return sModelNames3D[model];
    }
    return "NORMAL";
}
}  // namespace

PlayerModelChanger2D3D::PlayerModelChanger2D3D(const al::LiveActor* player,
                                               const PlayerInput* input,
                                               PlayerModelHolder* modelHolder,
                                               const ActorDimensionKeeper* dimensionKeeper)
    : mPlayer(player), mInput(input), mIsChange(false), mIs2DModel(false),
      mIsDeathRequested(false), mModel(0), mRequestedModel(0), mModelActor(nullptr), mBlinkModelName(nullptr), mTargetModelName(nullptr),
      mModelHolder(modelHolder), mDimensionKeeper(dimensionKeeper), mBlinkTimer(0),
      mChangeCooldown(0) {
    mModelActor = mModelHolder->getCurrentModelActor();
    mModelActor->makeActorAlive();
    mIs2DModel = mModelHolder->isCurrentModelLabelSubString("2D");
    mIsChange = true;
}


// NON_MATCHING: complete model-blink and dimension-change behavior is recovered; remaining differences are branch/register shape around model-name selection and cooldown clamping.
void PlayerModelChanger2D3D::update(bool isChangeEnabled) {
    if (mModel != mRequestedModel) {
        mBlinkModelName = ::getModelName(mModel, mIs2DModel);
        mModel = mRequestedModel;
        mTargetModelName = ::getModelName(mModel, mIs2DModel);
        mBlinkTimer = mIsDeathRequested ? 1 : 63;
    }

    if (mBlinkTimer >= 1) {
        mBlinkTimer += mBlinkTimer == 1 ? -1 : -2;
        if ((mBlinkTimer & 7) < (mBlinkTimer >> 3))
            mModelHolder->changeModel(mBlinkModelName);
        else
            mModelHolder->changeModel(mTargetModelName);
        changeModel(mModelHolder->getCurrentModelActor());

        if (mBlinkTimer == 0) {
            mModelHolder->changeModel(mTargetModelName);
            changeModel(mModelHolder->getCurrentModelActor());
        }
        return;
    }

    s32 changeCooldown = mChangeCooldown - 1;
    mIsChange = false;
    mChangeCooldown = changeCooldown < 0 ? 0 : changeCooldown;
    if (mChangeCooldown > 0)
        return;

    if (!((mIs2DModel && mInput->isTriggerChange3D()) ||
          (!mIs2DModel && mInput->isTriggerChange2D())) ||
        !isChangeEnabled)
        return;

    mChangeCooldown = 60;
    bool was2D = mIs2DModel;
    mIs2DModel = !mIs2DModel;
    if (was2D)
        al::startHitReaction(mPlayer, "3D変身");
    else
        al::startHitReaction(mPlayer, "2D変身");
    mModelHolder->changeModel(::getModelName(mModel, mIs2DModel));
    changeModel(mModelHolder->getCurrentModelActor());
}

// NON_MATCHING: labels and fallback are exact, but the target uses cmp #3 with separate flag/range branches while this shared-fallback form uses cmp #2/b.hi; next hypothesis is the original switch/table source form.
const char* PlayerModelChanger2D3D::getModelName() {
    s32 model = mModel;
    bool is2D = mIs2DModel;
    if ((u32)model < 3) {
        if (is2D)
            return sModelNames2D[model];
        return sModelNames3D[model];
    }
    return "NORMAL";
}

void PlayerModelChanger2D3D::changeModel(al::LiveActor* modelActor) {
    mModelActor->makeActorDead();
    al::showModelIfHide(modelActor);
    modelActor->makeActorAlive();
    mModelActor = modelActor;
    mIsChange = true;
}


void PlayerModelChanger2D3D::updateDead() {
    if (!mIs2DModel)
        return;

    mIs2DModel = false;
    mModelHolder->changeModel(::getModelName(mModel, false));
    changeModel(mModelHolder->getCurrentModelActor());
}

// NON_MATCHING: behavior and control flow match, with one extra bool-normalization instruction on return; next hypothesis is the original integer-width declaration or call-site type.
bool PlayerModelChanger2D3D::requestDamage() {
    bool result = isMini();
    if (!result)
        mRequestedModel = mModel - 1;
    return result;
}

bool PlayerModelChanger2D3D::requestKinokoSuper() {
    if (mModel > 0)
        return false;
    mRequestedModel = 1;
    return true;
}

bool PlayerModelChanger2D3D::requestFireFlower() {
    if (mModel > 1)
        return false;
    mRequestedModel = 2;
    return true;
}

bool PlayerModelChanger2D3D::requestMini() {
    if (mModel == 0)
        return false;
    mRequestedModel = 0;
    return true;
}

bool PlayerModelChanger2D3D::requestDeath() {
    if (!is2DModel())
        return false;
    mRequestedModel = 0;
    mIsDeathRequested = true;
    return true;
}

void PlayerModelChanger2D3D::syncPose(const al::LiveActor* actor) {
    al::copyPose(mModelActor, actor);
    if (mIsChange)
        mModelActor->calcAnim();
}

bool PlayerModelChanger2D3D::isFireFlower() const {
    return mModel == 2;
}

bool PlayerModelChanger2D3D::isMini() const {
    return mModel == 0;
}

bool PlayerModelChanger2D3D::isChange() const {
    return mIsChange;
}

bool PlayerModelChanger2D3D::is2DModel() const {
    return mIs2DModel;
}

bool PlayerModelChanger2D3D::isHiddenModel() const {
    return false;
}

bool PlayerModelChanger2D3D::isHiddenShadowMask() const {
    return false;
}

void PlayerModelChanger2D3D::resetPosition() {}
void PlayerModelChanger2D3D::hideModel() {}
void PlayerModelChanger2D3D::hideSilhouette() {}
void PlayerModelChanger2D3D::hideShadowMask() {}
void PlayerModelChanger2D3D::showModel() {}
void PlayerModelChanger2D3D::showSilhouette() {}
void PlayerModelChanger2D3D::showShadowMask() {}
