#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
struct ActorInitInfo;
class HitSensor;
class IUseNerve;
class Nerve;
class PlacementInfo;
class SensorMsg;
}  // namespace al

namespace sead {
template <typename T>
struct Quat;
using Quatf = Quat<f32>;
template <typename T>
struct Vector3;
using Vector3f = Vector3<f32>;
}  // namespace sead

class ActorStateReactionBase;
class DigPoint;
class Shibaken;

namespace ShibakenFunction {

void addFallVelocityToGround(Shibaken* shibaken, f32 gravity);
bool isPlayingMoveAction(const Shibaken* shibaken);
void limitFallVelocityOnGround(Shibaken* shibaken);
bool isGroundNormal(const sead::Vector3f& normal, const Shibaken* shibaken);
f32 getJumpStartSpeedV(const Shibaken* shibaken);
f32 getJumpGravityAccel(const Shibaken* shibaken);
f32 getJumpAirFriction(const Shibaken* shibaken);
bool executeReactionNerve(al::HostStateBase<Shibaken>* state);
bool tryStartReaction(al::IUseNerve* user, ActorStateReactionBase* reaction,
                      const al::Nerve* nerve, const al::SensorMsg* msg,
                      al::HitSensor* other, al::HitSensor* self);
bool checkStopChaseByFaceWall(const Shibaken* shibaken);
bool checkStopChaseByFaceWall(const Shibaken* shibaken, const sead::Vector3f& dir);
void addFallVelocityToGroundAndFitPoseOnGround(Shibaken* shibaken, f32 gravity);
bool chaseToPlayerAndTryStop(Shibaken* shibaken);
void chaseToTargetRun(Shibaken* shibaken, const sead::Vector3f& target);
void chaseToTargetWalk(Shibaken* shibaken, const sead::Vector3f& target);
void chaseToTargetWalkSniff(Shibaken* shibaken, const sead::Vector3f& target);
void chaseToTarget(Shibaken* shibaken, const sead::Vector3f& target, f32 speed,
                   bool limitToTarget, bool skipTurn);
bool tryStartJump(al::IUseNerve* user, const Shibaken* shibaken, const al::Nerve* nerve);
bool tryStartJump(al::HostStateBase<Shibaken>* state, const al::Nerve* nerve);
bool executeFindTurnNerve(al::HostStateBase<Shibaken>* state, const sead::Vector3f& target,
                          sead::Quatf* quatA, sead::Quatf* quatB);

}  // namespace ShibakenFunction

struct ShibakenDigPointLocater {
    DigPoint* point;
    ShibakenDigPointLocater** children;
    s32 childCount;
    bool isValid;
};

void initShibakenDigPointLocater(ShibakenDigPointLocater* locater,
                                 const al::ActorInitInfo& info,
                                 const al::PlacementInfo& placementInfo);
void updateShibakenDigPointLocaterHintTrans(ShibakenDigPointLocater* locater,
                                            const sead::Vector3f& trans);
