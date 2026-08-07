#include "Player/HackerStateJump.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Player/HackerActionAirMoveControl.h"
#include "Player/HackerStateConst.h"
#include "Player/PlayerActionFunction.h"
#include "Player/PlayerActionVelocityControl.h"
#include "Util/ObjUtil.h"

namespace {
NERVE_IMPL(HackerStateJump, Jump);
NERVE_IMPL(HackerStateJump, JumpStart);
NERVES_MAKE_STRUCT(HackerStateJump, Jump, JumpStart);
}  // namespace

HackerStateJump::HackerStateJump(al::LiveActor* actor, IUsePlayerHack** playerHack, bool is2D,
                                 bool isHack)
    : HackerStateBase("待機", actor, playerHack), mJumpAnim("Jump"), mJumpStartAnim("JumpStart"),
      mConst(nullptr), mAirMoveControl(nullptr), _48(0), mJumpSpeed(0.0f),
      mJumpPowerMin(0.0f), mJumpPowerMax(0.0f), _58(0.0f), _5c(0.0f), _60(0.0f), _64(0.0f),
      _68(0.0f), _6c(0.0f) {
    mConst = new HackerStateConst();
    _48 = mConst->_58;
    _6c = mConst->_50;
    mJumpPowerMin = mConst->_3c;
    mJumpPowerMax = mConst->_40;
    _58 = mConst->_34;
    _5c = mConst->_38;
    _60 = mConst->_8;
    _64 = mConst->_c;
    _68 = mConst->_10;

    HackerActionAirMoveControl* airMoveControl =
        new HackerActionAirMoveControl(actor, is2D, isHack);
    mAirMoveControl = airMoveControl;
    airMoveControl->set_28(true);
    {
        HackerStateConst* stateConst = mConst;
        s32 value30 = stateConst->_30bits;
        airMoveControl->set_2a(true);
        airMoveControl->set_2c_30(0, value30);
        airMoveControl->setupCollideWallScaleVelocity(stateConst->_5c, 1.0f, stateConst->_48);
    }
    mAirMoveControl->setPlayerHack(playerHack);
    mAirMoveControl->setupTurn(mConst->_14, mConst->_18, mConst->_1c, mConst->_20,
                               mConst->_24, mConst->_28, mConst->_2c);
    initNerve(&NrvHackerStateJump.Jump, 0);
}

void HackerStateJump::appear() {
    al::NerveStateBase::appear();
    al::LiveActor* actor = mActor;
    al::scaleVelocityInertiaWallHit(actor, mConst->_5c, 1.0f, mConst->_48);

    PlayerActionVelocityControl velocityControl(actor, nullptr);
    mJumpSpeed = PlayerActionFunction::calcJumpSpeed(velocityControl.getVelocityFront().length(),
                                                     mConst->_44, mConst->_48,
                                                     mJumpPowerMin, mJumpPowerMax);
    mAirMoveControl->setup(_5c, _58, _48, mJumpSpeed, _6c, 0, mConst->_54, mConst->_0,
                           mConst->_44, mConst->_4, _60, _64, _68);

    if (al::isExistAction(mActor, mJumpStartAnim)) {
        al::startAction(actor, mJumpStartAnim);
        al::setNerve(this, &NrvHackerStateJump.JumpStart);
    } else {
        al::startAction(actor, mJumpAnim);
        al::setNerve(this, &NrvHackerStateJump.Jump);
    }
}

void HackerStateJump::setupTurnControlParam(f32 a, f32 b, f32 c, f32 d, s32 e, s32 f, s32 g) {
    mAirMoveControl->setupTurn(a, b, c, d, e, f, g);
}

void HackerStateJump::setupForceJumpExtend(bool value) {
    mAirMoveControl->setForceJumpExtend(value);
}

bool HackerStateJump::isHoldJumpExtend() const {
    return mAirMoveControl->isHoldJumpExtend();
}

void HackerStateJump::updateJumpPower(f32 jumpPowerMin, f32 jumpPowerMax) {
    mJumpPowerMin = jumpPowerMin;
    mJumpPowerMax = jumpPowerMax;

    PlayerActionVelocityControl velocityControl(mActor, nullptr);
    mJumpSpeed = PlayerActionFunction::calcJumpSpeed(velocityControl.getVelocityFront().length(),
                                                     mConst->_44, mConst->_48,
                                                     mJumpPowerMin, mJumpPowerMax);
    mAirMoveControl->setup(_5c, _58, _48, mJumpSpeed, _6c, 0, mConst->_54, mConst->_0,
                           mConst->_44, mConst->_4, _60, _64, _68);
}

void HackerStateJump::exeJumpStart() {
    updateJump();
    if (al::isActionEnd(mActor)) {
        al::startAction(mActor, mJumpAnim);
        al::setNerve(this, &NrvHackerStateJump.Jump);
    }
}

void HackerStateJump::updateJump() {
    al::LiveActor* actor = mActor;
    if (al::isCollidedCeiling(actor)) {
        rs::reflectCeiling(actor, 0.0f);
        mAirMoveControl->setExtendFrame(0);
    }

    mAirMoveControl->update();
    if (!al::isOnGround(actor, 0))
        return;

    sead::Vector3f velocityH = {0.0f, 0.0f, 0.0f};
    sead::Vector3f velocityV = {0.0f, 0.0f, 0.0f};
    al::separateVelocityHV(&velocityH, &velocityV, mActor);
    f32 speedH = velocityH.length();
    speedH = speedH < mAirMoveControl->get_4c() ? speedH : mAirMoveControl->get_4c();
    al::limitLength(&velocityH, velocityH, speedH);
    al::setVelocity(mActor, velocityV + velocityH);
    kill();
}

void HackerStateJump::exeJump() {
    updateJump();
}

void HackerStateJump::doLanding() {
    sead::Vector3f velocityH = {0.0f, 0.0f, 0.0f};
    sead::Vector3f velocityV = {0.0f, 0.0f, 0.0f};
    al::separateVelocityHV(&velocityH, &velocityV, mActor);
    f32 speedH = velocityH.length();
    speedH = speedH < mAirMoveControl->get_4c() ? speedH : mAirMoveControl->get_4c();
    al::limitLength(&velocityH, velocityH, speedH);
    al::setVelocity(mActor, velocityV + velocityH);
}
