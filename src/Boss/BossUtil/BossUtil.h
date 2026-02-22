#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
}  // namespace al

namespace rs {

void startBossBattle(const al::LiveActor* actor, s32 worldId);
void endBossBattle(const al::LiveActor* actor, s32 worldId);
bool isAlreadyShowDemoBossBattleStart(const al::LiveActor* actor, s32 worldId, s32 level);
void saveShowDemoBossBattleStart(const al::LiveActor* actor, s32 worldId, s32 level);

}  // namespace rs
