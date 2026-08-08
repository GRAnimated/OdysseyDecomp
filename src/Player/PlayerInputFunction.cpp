#include "Player/PlayerInputFunction.h"

#include <math/seadVector.h>

#include "Library/Controller/InputFunction.h"
#include "Library/LiveActor/LiveActor.h"

bool PlayerInputFunction::isSeparatePlaySingleJoy(const al::LiveActor* actor, s32 port) {
    return rs::isSeparatePlay(actor) && al::isPadTypeJoySingle(port);
}

bool PlayerInputFunction::isTriggerJump(const al::LiveActor* actor, s32 port) {
    return al::isPadTriggerA(port) || al::isPadTriggerB(port);
}

bool PlayerInputFunction::isHoldJump(const al::LiveActor* actor, s32 port) {
    return al::isPadHoldA(port) || al::isPadHoldB(port);
}

bool PlayerInputFunction::isReleaseJump(const al::LiveActor* actor, s32 port) {
    return al::isPadReleaseA(port) || al::isPadReleaseB(port);
}

bool PlayerInputFunction::isTriggerAction(const al::LiveActor* actor, s32 port) {
    if (isSeparatePlaySingleJoy(actor, port))
        return al::isPadTriggerY(port);
    return al::isPadTriggerX(port) || al::isPadTriggerY(port);
}

bool PlayerInputFunction::isHoldAction(const al::LiveActor* actor, s32 port) {
    if (isSeparatePlaySingleJoy(actor, port))
        return al::isPadHoldY(port);
    return al::isPadHoldX(port) || al::isPadHoldY(port);
}

bool PlayerInputFunction::isReleaseAction(const al::LiveActor* actor, s32 port) {
    if (isSeparatePlaySingleJoy(actor, port))
        return al::isPadReleaseY(port);
    return al::isPadReleaseX(port) || al::isPadReleaseY(port);
}

bool PlayerInputFunction::isTriggerSubAction(const al::LiveActor* actor, s32 port) {
    if (isSeparatePlaySingleJoy(actor, port))
        return al::isPadTriggerL(port) || al::isPadTriggerR(port);
    return al::isPadTriggerZL(port) || al::isPadTriggerZR(port);
}

bool PlayerInputFunction::isHoldSubAction(const al::LiveActor* actor, s32 port) {
    if (isSeparatePlaySingleJoy(actor, port))
        return al::isPadHoldL(port) || al::isPadHoldR(port);
    return al::isPadHoldZL(port) || al::isPadHoldZR(port);
}

bool PlayerInputFunction::isReleaseSubAction(const al::LiveActor* actor, s32 port) {
    if (isSeparatePlaySingleJoy(actor, port))
        return al::isPadReleaseL(port) || al::isPadReleaseR(port);
    return al::isPadReleaseZL(port) || al::isPadReleaseZR(port);
}

bool PlayerInputFunction::isHoldBalloonSet(const al::LiveActor* actor, s32 port) {
    if (isSeparatePlaySingleJoy(actor, port))
        return al::isPadHoldL(port) && al::isPadHoldR(port);
    return al::isPadHoldZL(port) && al::isPadHoldZR(port);
}

bool PlayerInputFunction::isTriggerTalk(const al::LiveActor* actor, s32 port) {
    return al::isPadTriggerA(port);
}

bool PlayerInputFunction::isTriggerStartWorldWarp(const al::LiveActor* actor, s32 port) {
    return al::isPadTriggerA(port);
}

bool PlayerInputFunction::isTriggerCancelWorldWarp(const al::LiveActor* actor, s32 port) {
    if (al::isPadTriggerB(port) || al::isPadTriggerY(port))
        return true;
    if (isSeparatePlaySingleJoy(actor, port))
        return false;
    return al::isPadTriggerX(port);
}

sead::Vector2f PlayerInputFunction::getMoveInputStick(const al::LiveActor* actor, s32 port,
                                                        s32 stick) {
    if (!rs::isSeparatePlay(actor) || !al::isPadTypeJoySingle(port))
        return al::getLeftStick(port);

    if (rs::isSeparatePlay(actor) && al::isPadTypeJoySingle(port)) {
        s32 isHoldX = al::isPadHoldX(port);
        if (stick)
            return al::getLeftStick(port);
        if ((isHoldX & 1) == 0)
            return al::getLeftStick(port);
        return sead::Vector2f::zero;
    }

    if (!al::isPadHoldL(port) && !al::isPadHoldR(port))
        return al::getLeftStick(port);

    return sead::Vector2f::zero;
}

bool PlayerInputFunction::isHoldCameraReset(const al::LiveActor* actor, s32 port, s32 controller) {
    if (isSeparatePlaySingleJoy(actor, port)) {
        bool isHoldX = al::isPadHoldX(port);
        return (controller == 0) & isHoldX;
    }
    return al::isPadHoldL(port) || al::isPadHoldR(port);
}

f32 PlayerInputFunction::getRadiconInputSteeringValue(const al::LiveActor* actor, s32 port) {
    if (!isSeparatePlaySingleJoy(actor, port))
        return al::getRightStick(port).x;
    if (al::isPadHoldY(port))
        return -1.0f;
    if (al::isPadHoldA(port))
        return 1.0f;
    return 0.0f;
}

u32 PlayerInputFunction::getNoInputJudgeKeyMask(const al::LiveActor* actor, s32 port) {
    u32 keyMask = 0xF1FFF;
    if (rs::isSeparatePlay(actor) && al::isPadTypeJoySingle(port))
        keyMask = 0xF7FF7;
    return keyMask;
}

bool PlayerInputFunction::isInputLeftStickNoCameraMove(const al::LiveActor* actor, s32 port,
                                                        f32 threshold) {
    if (al::getLeftStick(port).length() < threshold)
        return false;
    if (rs::isSeparatePlay(actor) && al::isPadTypeJoySingle(port) && al::isPadHoldX(port))
        return false;
    return true;
}

bool PlayerInputFunction::isTriggerCameraReset(const al::LiveActor* actor, s32 port) {
    if (isSeparatePlaySingleJoy(actor, port))
        return al::isPadTriggerX(port);
    return al::isPadTriggerL(port) || al::isPadTriggerR(port);
}

bool PlayerInputFunction::isTriggerSeparateCameraReset(const al::LiveActor* actor, s32 port,
                                                        s32 controller) {
    if (isSeparatePlaySingleJoy(actor, port)) {
        if (controller)
            return al::isPadReleaseX(port);
        return false;
    }
    return al::isPadTriggerL(port) || al::isPadTriggerR(port);
}

bool PlayerInputFunction::isTriggerCameraSubjective(const al::LiveActor* actor, s32 port) {
    if (isSeparatePlaySingleJoy(actor, port))
        return false;
    return al::isPadTriggerPressRightStick(port);
}

bool PlayerInputFunction::isHoldCameraZoom(const al::LiveActor* actor, s32 port) {
    if (isSeparatePlaySingleJoy(actor, port))
        return false;
    return al::isPadHoldZL(port) || al::isPadHoldZR(port);
}

bool PlayerInputFunction::isHoldCameraSnapShotZoomIn(const al::LiveActor* actor, s32 port) {
    if (isSeparatePlaySingleJoy(actor, port))
        return false;
    return al::isPadHoldX(port);
}

bool PlayerInputFunction::isHoldCameraSnapShotZoomOut(const al::LiveActor* actor, s32 port) {
    if (isSeparatePlaySingleJoy(actor, port))
        return false;
    return al::isPadHoldA(port);
}

bool PlayerInputFunction::isHoldCameraSnapShotRollLeft(const al::LiveActor* actor, s32 port) {
    if (isSeparatePlaySingleJoy(actor, port))
        return false;
    return al::isPadHoldZL(port);
}

bool PlayerInputFunction::isHoldCameraSnapShotRollRight(const al::LiveActor* actor, s32 port) {
    if (isSeparatePlaySingleJoy(actor, port))
        return false;
    return al::isPadHoldZR(port);
}

sead::Vector2f PlayerInputFunction::getCameraMoveInput(const al::LiveActor* actor, s32 port,
                                                          s32 camera, bool isSeparate) {
    const sead::Vector2f* input;
    bool clearY = false;

    if (rs::isSeparatePlay(actor) && al::isPadTypeJoySingle(port)) {
        if (rs::isSeparatePlay(actor) && al::isPadTypeJoySingle(port)) {
            s32 isHoldX = al::isPadHoldX(port);
            if (!camera && isHoldX) {
                input = &al::getLeftStick(port);
                return *input;
            }
        } else if (al::isPadHoldL(port) || al::isPadHoldR(port)) {
            input = &al::getLeftStick(port);
            return *input;
        }

        if (isSeparate && al::isPadHoldPressLeftStick(port)) {
            input = &al::getLeftStick(port);
            clearY = true;
        } else {
            input = &sead::Vector2f::zero;
        }
    } else {
        input = &al::getRightStick(port);
    }

    return sead::Vector2f(input->x, clearY ? 0.0f : input->y);
}
