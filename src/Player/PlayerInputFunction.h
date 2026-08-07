#pragma once

#include <basis/seadTypes.h>

namespace al {
class IUseSceneObjHolder;
class LiveActor;
}  // namespace al

class PlayerInputFunction {
public:
    static bool isSeparatePlaySingleJoy(const al::LiveActor* actor, s32 port);
    static bool isTriggerJump(const al::LiveActor* actor, s32 port);
    static bool isHoldJump(const al::LiveActor* actor, s32 port);
    static bool isReleaseJump(const al::LiveActor* actor, s32 port);
    static bool isTriggerAction(const al::LiveActor* actor, s32 port);
    static bool isHoldAction(const al::LiveActor* actor, s32 port);
    static bool isReleaseAction(const al::LiveActor* actor, s32 port);
    static bool isTriggerSubAction(const al::LiveActor* actor, s32 port);
    static bool isHoldSubAction(const al::LiveActor* actor, s32 port);
    static bool isReleaseSubAction(const al::LiveActor* actor, s32 port);
    static bool isHoldBalloonSet(const al::LiveActor* actor, s32 port);
    static bool isTriggerTalk(const al::LiveActor* actor, s32 port);
    static bool isTriggerStartWorldWarp(const al::LiveActor* actor, s32 port);
    static bool isTriggerCancelWorldWarp(const al::LiveActor* actor, s32 port);
    static f32 getMoveInputStick(const al::LiveActor* actor, s32 port, s32 stick);
    static bool isHoldCameraReset(const al::LiveActor* actor, s32 port, s32 controller);
    static f32 getRadiconInputSteeringValue(const al::LiveActor* actor, s32 port);
    static u64 getNoInputJudgeKeyMask(const al::LiveActor* actor, s32 port);
    static bool isInputLeftStickNoCameraMove(const al::LiveActor* actor, s32 port,
                                             f32 threshold);
    static bool isTriggerCameraReset(const al::LiveActor* actor, s32 port);
    static bool isTriggerSeparateCameraReset(const al::LiveActor* actor, s32 port,
                                             s32 controller);
    static bool isTriggerCameraSubjective(const al::LiveActor* actor, s32 port);
    static bool isHoldCameraZoom(const al::LiveActor* actor, s32 port);
    static bool isHoldCameraSnapShotZoomIn(const al::LiveActor* actor, s32 port);
    static bool isHoldCameraSnapShotZoomOut(const al::LiveActor* actor, s32 port);
    static bool isHoldCameraSnapShotRollLeft(const al::LiveActor* actor, s32 port);
    static bool isHoldCameraSnapShotRollRight(const al::LiveActor* actor, s32 port);
    static f32 getCameraMoveInput(const al::LiveActor* actor, s32 port, s32 camera,
                                  bool isSeparate);
};

namespace rs {

bool isSeparatePlay(const al::IUseSceneObjHolder* sceneObjHolder);

}  // namespace rs

static_assert(sizeof(PlayerInputFunction) == 0x1);
