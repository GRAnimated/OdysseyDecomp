#pragma once

#include <math/seadVector.h>

namespace al {
class LiveActor;
class HitSensor;
class PlayerHolder;
class PadRumbleKeeper;

s32 getPlayerNumMax(const PlayerHolder* holder);
s32 getAlivePlayerNum(const PlayerHolder* holder);
LiveActor* getPlayerActor(const PlayerHolder* holder, s32 index);
const sead::Vector3f& getPlayerPos(const PlayerHolder* holder, s32 index);
LiveActor* tryGetPlayerActor(const PlayerHolder* holder, s32 index);
bool isPlayerDead(const PlayerHolder* holder, s32 index);
bool isPlayerAreaTarget(const PlayerHolder* holder, s32 index);
LiveActor* tryFindAlivePlayerActorFirst(const PlayerHolder* holder);
LiveActor* findAlivePlayerActorFirst(const PlayerHolder* holder);
const sead::Vector3f& findNearestPlayerPos(const LiveActor* actor);
LiveActor* tryFindNearestPlayerActor(const LiveActor* actor);
}  // namespace al

namespace alPlayerFunction {
void registerPlayer(al::LiveActor*, al::PadRumbleKeeper*);
bool isFullPlayerHolder(al::LiveActor*);
s32 findPlayerHolderIndex(const al::LiveActor*);
s32 findPlayerHolderIndex(const al::HitSensor*);
bool isPlayerActor(const al::LiveActor*);
bool isPlayerActor(const al::HitSensor*);
}  // namespace alPlayerFunction
