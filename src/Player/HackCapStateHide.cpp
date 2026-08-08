#include "Player/HackCapStateHide.h"

#include <math/seadQuat.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorResourceFunction.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Obj/PartsModel.h"
#include "Library/Shadow/ActorShadowUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Yaml/ByamlIter.h"
#include "Library/Yaml/ByamlUtil.h"

#include "Player/HackCapFunction.h"
#include "Player/HackCapJointControlKeeper.h"
#include "Player/PlayerColliderHackCap.h"
#include "Player/PlayerInput.h"
#include "Player/PlayerSeparateCapFlag.h"
#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerUtil.h"

namespace {
NERVE_IMPL(HackCapStateHide, Hide)
NERVE_IMPL(HackCapStateHide, SeparateWait)
NERVES_MAKE_STRUCT(HackCapStateHide, Hide, SeparateWait)
}  // namespace

HackCapStateHide::HackCapStateHide(al::LiveActor* actor, PlayerColliderHackCap* collider,
                                   const al::LiveActor* player,
                                   const PlayerSeparateCapFlag* separateCapFlag,
                                   const PlayerInput* input, al::PartsModel* partsModel,
                                   HackCapJointControlKeeper* jointControlKeeper)
    : ActorStateBase("隠れ", actor), mCollider(collider), mPlayer(player),
      mSeparateCapFlag(separateCapFlag), mInput(input), mPartsModel(partsModel),
      mSubActor(nullptr), mJointControlKeeper(jointControlKeeper), _58{0.0f, 0.0f, 0.0f}, _64(false), _68{1.0f, 1.0f, 1.0f} {
    mSubActor = static_cast<al::PartsModel*>(al::getSubActor(actor, "目(おすそ分け待機)"));
    if (al::isExistModelResourceYaml(actor, "InitSeparateHideScale", nullptr)) {
        al::ByamlIter iter(al::getModelResourceYaml(actor, "InitSeparateHideScale", nullptr));
        al::tryGetByamlV3f(&_68, iter, "SeparateHide");
    }
    initNerve(&NrvHackCapStateHide.Hide, 0);
}

void HackCapStateHide::appear() {
    al::NerveStateBase::appear();
    _58.set(0.0f, 0.0f, 0.0f);
    const u32 flags = mSeparateCapFlag->getRawFlags();
    if ((flags & 0xFF0000) != 0 || (flags & 0xFF) == 0)
        al::setNerve(this, &NrvHackCapStateHide.Hide);
    else
        al::setNerve(this, &NrvHackCapStateHide.SeparateWait);
}

void HackCapStateHide::kill() {
    mSubActor->offSyncAppearAndHide();
    mSubActor->kill();
    mPartsModel->onSyncAppearAndHide();
    al::NerveStateBase::kill();
}

bool HackCapStateHide::update() {
    const PlayerSeparateCapFlag* flag = mSeparateCapFlag;
    const bool isHide = al::isNerve(this, &NrvHackCapStateHide.Hide);
    const u32 rawFlags = flag->getRawFlags();
    const bool isSeparate = (rawFlags & 0xFF0000) == 0 && (rawFlags & 0xFF) != 0;
    if (isHide) {
        if (isSeparate && flag->isSeparateCapLocal() && !flag->isPuppetable())
            al::setNerve(this, &NrvHackCapStateHide.SeparateWait);
    } else if (!isSeparate || !flag->isSeparateCapLocal() || flag->isPuppetable()) {
        al::setNerve(this, &NrvHackCapStateHide.Hide);
    }
    return al::NerveStateBase::update();
}

bool HackCapStateHide::isSeparateMode() const {
    return !isDead() && !al::isNerve(this, &NrvHackCapStateHide.Hide);
}

void HackCapStateHide::cancelSeparateMode() {
    al::setNerve(this, &NrvHackCapStateHide.Hide);
}

void HackCapStateHide::calcSeparateThrowOffset(sead::Vector3f* offset) const {
    offset->setSub(al::getTrans(mActor), al::getTrans(mPlayer));
}

void HackCapStateHide::exeHide() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        mJointControlKeeper->reset();
        al::tryStartActionIfNotPlaying(actor, "WaitSeparate");
        al::setScaleAll(actor, 1.0f);
        al::hideModelIfShow(actor);
        al::invalidateShadow(actor);
        al::offCollide(actor);
        al::setVelocityZero(actor);
    }

    const u32 flags = mSeparateCapFlag->getRawFlags();
    if ((flags & 0xFF0000) == 0 && (flags & 0xFF) != 0) {
        tryForceFollowSeparate();
        HackCapFunction::updateSeparateWaitMove(actor, &_58, mCollider, mPlayer, mInput, 30.0f,
                                                0.0f, 170.0f,
                                                mSeparateCapFlag->getSeparateCapLocalOffset());
    }
}

void HackCapStateHide::tryForceFollowSeparate() {
    if (_64) {
        _64 = false;
        const u32 flags = mSeparateCapFlag->getRawFlags();
        if ((flags & 0xFF0000) == 0 && (flags & 0xFF) != 0) {
            al::LiveActor* actor = mActor;
            const f32 radius = mCollider->getColliderRadius();
            sead::Vector3f groundUp{0.0f, 0.0f, 0.0f};
            rs::calcPlayerGroundPoseUp(&groundUp, actor);
            f32 ceilingSpace = 0.0f;
            rs::tryCalcPlayerCeilingSpace(&ceilingSpace, mPlayer, 170.0f, radius);

            sead::Quatf quat = sead::Quatf::unit;
            al::calcQuat(&quat, mPlayer);
            sead::Vector3f localOffset;
            localOffset.setRotated(quat, mSeparateCapFlag->getSeparateCapLocalOffset());
            al::resetPosition(actor,
                              al::getTrans(mPlayer) + ceilingSpace * groundUp + localOffset);
            rs::resetCollisionExpandCheck(mCollider);
        }
    }
}

void HackCapStateHide::exeSeparateWait() {
    al::LiveActor* actor = mActor;
    if (al::isFirstStep(this)) {
        mJointControlKeeper->reset();
        al::tryStartActionIfNotPlaying(actor, "WaitSeparate");
        al::setScale(actor, _68);
        al::showModelIfHide(actor);
        al::validateShadow(actor);
        al::onCollide(actor);
        _64 = true;
        al::setShadowMaskIntensity(actor, "本体", 0.8f);
        mPartsModel->offSyncAppearAndHide();
        mPartsModel->kill();
        mSubActor->onSyncAppearAndHide();
        al::startAction(mSubActor, "Wait");
    }
    tryForceFollowSeparate();
    HackCapFunction::updateSeparateWaitMove(actor, &_58, mCollider, mPlayer, mInput, 30.0f,
                                            0.0f, 170.0f,
                                            mSeparateCapFlag->getSeparateCapLocalOffset());
}

HackCapStateHide::~HackCapStateHide() = default;
