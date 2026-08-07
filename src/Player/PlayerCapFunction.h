#pragma once

#include <math/seadVector.h>

class HackCap;
class PlayerAnimator;

namespace al {
class LiveActor;
}
struct HackObjInfo;

namespace CapFunction {

void putOnCapPlayer(HackCap* hackCap, PlayerAnimator* animator);
void endHack(HackCap* hackCap, PlayerAnimator* animator);
const HackObjInfo* getHackObjInfo(HackCap* hackCap);

}  // namespace CapFunction

namespace PlayerCapFunction {
bool tryCalcHackCapThrowInputNormal(sead::Vector3f*, const al::LiveActor*);
}
