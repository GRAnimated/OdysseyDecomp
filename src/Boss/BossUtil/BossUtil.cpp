#include "Boss/BossUtil/BossUtil.h"

#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Placement/PlacementFunction.h"

#include "Boss/BarrierField.h"
#include "Sequence/GameSequenceInfo.h"
#include "System/BossSaveData.h"
#include "System/CapMessageBossData.h"
#include "System/GameDataFile.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolder.h"
#include "System/GameDataHolderAccessor.h"
#include "System/GameDataUtil.h"

namespace rs {

BarrierField* tryCreateBarrierField(const al::ActorInitInfo& info) {
    if (al::calcLinkChildNum(info, "BarrierField") < 1)
        return nullptr;
    auto* barrierField = new BarrierField("ボス結界");
    al::initLinksActor(barrierField, info, "BarrierField", 0);
    barrierField->makeActorDead();
    return barrierField;
}

void updateEyeMove(al::LiveActor* actor, const sead::Vector3f& target, f32 maxAngle,
                   const char* animName) {
    f32 angle = al::calcAngleToTargetH(actor, target);
    if (angle > maxAngle)
        angle = 180.0f - angle;
    else if (angle < -maxAngle)
        angle = -180.0f - angle;
    f32 frameMax = al::getMtsAnimFrameMax(actor, animName);
    f32 frame = al::lerpValue(angle, -maxAngle, maxAngle, 0.0f, frameMax);
    al::startMtsAnimAndSetFrameAndStop(actor, animName, frame);
}

void resetEyeMove(al::LiveActor* actor, const char* animName) {
    f32 frameMax = al::getMtsAnimFrameMax(actor, animName);
    al::startMtsAnimAndSetFrameAndStop(actor, animName, frameMax * 0.5f);
}

void startBossBattle(const al::LiveActor* actor, s32 worldId) {
    CapMessageBossData* data = getCapMessageBossData(actor);
    data->incrementBossBattleCount(worldId);
    setSceneStatusBossBattle(actor);
}

// NON_MATCHING: eor+tbnz instead of tbz for bool checks
void endBossBattle(const al::LiveActor* actor, s32 worldId) {
    setSceneStatusBossBattleEndForPlayerAnim(actor);

    bool isSky = GameDataFunction::isWorldSky(actor);
    if ((worldId == 0 || worldId == 2) && isSky)
        setSceneStatusBossBattleEnd(actor);

    bool isMoon = GameDataFunction::isWorldMoon(actor);
    if (worldId == 12 && isMoon)
        setSceneStatusBossBattleEnd(actor);

    bool isSpecial1 = GameDataFunction::isWorldSpecial1(actor);
    if (worldId != 10 && isSpecial1)
        setSceneStatusBossBattleEnd(actor);
}

s32 getBossBattleDeadCount(const al::LiveActor* actor, s32 worldId) {
    CapMessageBossData* data = getCapMessageBossData(actor);
    return data->getBattleCount(worldId);
}

bool isAlreadyShowDemoBossBattleStart(const al::LiveActor* actor, s32 worldId, s32 level) {
    GameDataHolderAccessor accessor(actor);
    return accessor->getGameDataFile()->getBossSaveData()->isAlreadyShowDemoBossBattleStart(worldId,
                                                                                            level);
}

void saveShowDemoBossBattleStart(const al::LiveActor* actor, s32 worldId, s32 level) {
    GameDataHolderAccessor accessor(actor);
    accessor->getGameDataFile()->getBossSaveData()->showDemoBossBattleStart(worldId, level);
}

bool isAlreadyShowDemoBossBattleEndKoopaLv2(const al::LiveActor* actor) {
    GameDataHolderAccessor accessor(actor);
    return accessor->getGameDataFile()->getBossSaveData()->isAlreadyShowDemoBattleEndKoopaLv2();
}

void saveShowDemoBossBattleEndKoopaLv2(const al::LiveActor* actor) {
    GameDataHolderAccessor accessor(actor);
    accessor->getGameDataFile()->getBossSaveData()->saveDemoBattleEndKoopaLv2();
}

bool isAlreadyShowDemoMoonBasementCollapse(const al::LiveActor* actor) {
    GameDataHolderAccessor accessor(actor);
    return accessor->getGameDataFile()->getBossSaveData()->isAlreadyShowDemoMoonBasementCollapse();
}

void saveShowDemoMoonBasementCollapse(const al::LiveActor* actor) {
    GameDataHolderAccessor accessor(actor);
    accessor->getGameDataFile()->getBossSaveData()->saveShowDemoMoonBasementCollapse();
}

bool isAlreadyDeadGK(const al::LiveActor* actor, s32 worldId, s32 level) {
    GameDataHolderAccessor accessor(actor);
    return accessor->getGameDataFile()->getBossSaveData()->isAlreadyDeadGK(worldId, level);
}

void onAlreadyDeadGK(const al::LiveActor* actor, s32 worldId, s32 level) {
    GameDataHolderAccessor accessor(actor);
    accessor->getGameDataFile()->getBossSaveData()->onAlreadyDeadGK(worldId, level);
}

}  // namespace rs
