#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
}
class IJudge;
class PlayerColliderHackCap;
class PlayerInput;
class PlayerSeparateCapFlag;

namespace HackCapFunction {
void resetPositionAndCollision(al::LiveActor*, PlayerColliderHackCap*);
bool isKeepSeparateHold(const PlayerSeparateCapFlag*, IJudge*, bool);
f32 calcSeparateJumpGravity(bool*, s32*, bool*, s32*, const al::LiveActor*,
                            const PlayerColliderHackCap*, const PlayerInput*, f32, s32);
void updateSeparateWaitMove(al::LiveActor*, sead::Vector3f*, const PlayerColliderHackCap*,
                            const al::LiveActor*, const PlayerInput*, f32, f32, f32,
                            const sead::Vector3f&);
bool updateSeparateWaitJump(al::LiveActor*, sead::Vector3f*, f32*,
                            const PlayerColliderHackCap*, const al::LiveActor*,
                            const PlayerInput*, f32, f32, f32, f32,
                            const sead::Vector3f&);
}  // namespace HackCapFunction
