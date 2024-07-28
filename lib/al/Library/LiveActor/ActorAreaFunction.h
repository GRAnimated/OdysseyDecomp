#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class LiveActor;
class PlayerHolder;
class AreaObj;
class AreaObjGroup;
class AreaInitInfo;
class PlacementInfo;
class IUseAreaObj;
class ValidatorBase;

void tryFindAreaObjPlayerOne(const PlayerHolder*, const char*);
void tryFindAreaObjPlayerAll(const PlayerHolder*, const char*);
void tryGetAreaObjPlayerAll(const LiveActor*, AreaObjGroup const*);
bool isInAreaObjPlayerAll(const LiveActor*, const AreaObj*);
bool isInAreaObjPlayerAll(const LiveActor*, AreaObjGroup const*);
bool isInAreaObjPlayerAnyOne(const LiveActor*, const AreaObj*);
bool isInAreaObjPlayerAnyOne(const LiveActor*, AreaObjGroup const*);
void createAreaObj(const ActorInitInfo&, const char*);
void initAreaInitInfo(AreaInitInfo*, const ActorInitInfo&);
void createLinkArea(const ActorInitInfo&, const char*, const char*);
void initAreaInitInfo(AreaInitInfo*, const PlacementInfo&, const ActorInitInfo&);
void tryCreateLinkArea(const ActorInitInfo&, const char*, const char*);
void createLinkAreaGroup(const ActorInitInfo&, const char*, const char*, const char*);
void createLinkAreaGroup(LiveActor*, const ActorInitInfo&, const char*, const char*, const char*);
void tryFindAreaObj(const LiveActor*, const char*);
bool isInAreaObj(const LiveActor*, const char*);
bool isInAreaObjPlayerOne(const PlayerHolder*, const char*);
bool isInAreaObjPlayerAll(const PlayerHolder*, const char*);
bool isInAreaObjPlayerOneIgnoreAreaTarget(const PlayerHolder*, const char*);
bool isInDeathArea(const LiveActor*);
bool isInWaterArea(const LiveActor*);
bool isInPlayerControlOffArea(const LiveActor*);
void calcWaterSinkDepth(const LiveActor*);
void registerAreaHostMtx(const IUseAreaObj*, const sead::Matrix34f*, const ActorInitInfo&);
void registerAreaHostMtx(const IUseAreaObj*, const sead::Matrix34f*, const ActorInitInfo&,
                         const ValidatorBase&);
void registerAreaHostMtx(const LiveActor*, const ActorInitInfo&);
void registerAreaHostMtx(const LiveActor*, const ActorInitInfo&, const ValidatorBase&);
void registerAreaSyncHostMtx(const IUseAreaObj*, const sead::Matrix34f*, const ActorInitInfo&);
void registerAreaSyncHostMtx(const IUseAreaObj*, const sead::Matrix34f*, const ActorInitInfo&,
                             const ValidatorBase&);
void registerAreaSyncHostMtx(const LiveActor*, const ActorInitInfo&);
void registerAreaSyncHostMtx(const LiveActor*, const ActorInitInfo&, const ValidatorBase&);
void tryReviseVelocityInsideAreaObj(sead::Vector3f*, LiveActor*, AreaObjGroup*, const AreaObj*);
void tryCreateSwitchKeepOnAreaGroup(LiveActor*, const ActorInitInfo&);
void tryCreateSwitchOnAreaGroup(LiveActor*, const ActorInitInfo&);
}  // namespace al
