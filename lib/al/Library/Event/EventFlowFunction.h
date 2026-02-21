#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

namespace al {
struct ActorInitInfo;
class ByamlIter;
class EventFlowEventData;
class EventFlowExecutor;
class EventFlowNode;
class EventFlowNodeInitInfo;
class IEventFlowEventReceiver;
class IUseEventFlowData;
class IUseMessageSystem;
class IUseRail;
class LayoutInitInfo;
class LiveActor;

bool isInvalidUiCollisionCheck(const IUseEventFlowData* data);
bool isExistLookAtJointCtrl(const IUseEventFlowData* data);

s32 initRailMoveType(const LiveActor* actor, const ActorInitInfo& initInfo);
bool isRailMoveLoop(s32 type);
bool isRailMoveTurn(s32 type);
bool isRailMoveRestart(s32 type);

void initEventReceiver(EventFlowExecutor* executor, IEventFlowEventReceiver* receiver);
void initEventFlowNode(EventFlowNode* node, const EventFlowNodeInitInfo& initInfo);
void initEventQuery(EventFlowNode* node, const EventFlowNodeInitInfo& initInfo);
void registerEventCamera(EventFlowNode* node, const char* name);
void initEventNodeMessage(const char16** outMsg, const EventFlowNode* node,
                          const EventFlowNodeInitInfo& initInfo, const char* suffix);
const char* getParamIterKeyString(const EventFlowNodeInitInfo& initInfo, const char* key);
const char* getObjId(const EventFlowNodeInitInfo& initInfo);
const char* getPlacementStageName(const EventFlowNodeInitInfo& initInfo);
const ActorInitInfo& getActorInitInfo(const EventFlowNodeInitInfo& initInfo);
void makeParamMessageString(const char16** outMsg, const EventFlowNodeInitInfo& initInfo,
                            const char* key);
void initEventFlowMovement(EventFlowNode* node, const EventFlowNodeInitInfo& initInfo);
const LayoutInitInfo& getLayoutInitInfo(const EventFlowNodeInitInfo& initInfo);
bool tryGetParamIter(ByamlIter* outIter, const EventFlowNodeInitInfo& initInfo);
bool tryGetParamIterKeyInt(s32* outVal, const EventFlowNodeInitInfo& initInfo, const char* key);
bool tryGetParamIterKeyFloat(f32* outVal, const EventFlowNodeInitInfo& initInfo, const char* key);
bool tryGetParamIterKeyBool(bool* outVal, const EventFlowNodeInitInfo& initInfo, const char* key);
const char* tryGetParamIterKeyString(const EventFlowNodeInitInfo& initInfo, const char* key);
bool getParamIterKeyBool(const EventFlowNodeInitInfo& initInfo, const char* key);
s32 getParamIterKeyInt(const EventFlowNodeInitInfo& initInfo, const char* key);
const char* getNodeIterKeyString(const EventFlowNodeInitInfo& initInfo, const char* key);
const char16* getMessageString(IUseMessageSystem* msgSystem, const ByamlIter& iter);
const char* getParamMessageLabel(const EventFlowNodeInitInfo& initInfo, const char* key);
void restartEventMovement(EventFlowNode* node);
void stopEventMovement(EventFlowNode* node);
void recordActorFront(EventFlowNode* node);
const sead::Vector3f& getRecordActorFront(const EventFlowNode* node);
void sendEvent(EventFlowNode* node, const EventFlowEventData* eventData);
bool judgeQuery(EventFlowNode* node, const char* caseValue);
void invalidateClipping(EventFlowNode* node);
void validateClipping(EventFlowNode* node);
s32 getCaseEventNum(const EventFlowNode* node);
s32 getCaseEventNextId(const EventFlowNode* node, s32 index);
s32 findCaseEventNextId(const EventFlowNode* node, const char* caseValue);
const char16* getCaseEventMessage(const EventFlowNode* node, s32 index);
bool isTurnMovement(const EventFlowNode* node);
}  // namespace al
