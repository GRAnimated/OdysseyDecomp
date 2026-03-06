#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

namespace al {
struct ActorInitInfo;
class EventFlowExecutor;
class EventFlowMovement;
class IEventFlowQueryJudge;
class LiveActor;
class MessageTagDataHolder;
}  // namespace al

class TalkNpcActionAnimInfo;
class TalkNpcParam;

namespace rs {
TalkNpcParam* initTalkNpcParam(al::LiveActor*, const char*);
void initEventCharacterName(al::EventFlowExecutor*, const al::ActorInitInfo&, const char*);
void initEventParam(al::EventFlowExecutor*, const TalkNpcParam*, const char*);
void stopEventFlow(al::EventFlowExecutor*);
void restartEventFlow(al::EventFlowExecutor*);
void initEventCameraTalk(al::EventFlowExecutor*, const al::ActorInitInfo&, const char*, f32);
void initEventMovement(al::EventFlowExecutor*, al::EventFlowMovement*, const al::ActorInitInfo&);
void initEventMovementWait(al::EventFlowExecutor*, const al::ActorInitInfo&);
void initEventMovementRail(al::EventFlowExecutor*, const al::ActorInitInfo&);
void initEventMovementWander(al::EventFlowExecutor*, const al::ActorInitInfo&);
void initEventMovementTurnSeparate(al::EventFlowExecutor*, const al::ActorInitInfo&);
al::EventFlowExecutor* initEventFlow(al::LiveActor*, const al::ActorInitInfo&, const char*,
                                     const char*);
al::EventFlowExecutor* initEventFlowSuffix(al::LiveActor*, const al::ActorInitInfo&, const char*,
                                           const char*, const char*);
al::EventFlowExecutor* initEventFlowForSystem(al::LiveActor*, const al::ActorInitInfo&,
                                              const char*, const char*, const char*);
al::EventFlowExecutor* initEventFlowFromPlacementInfo(al::LiveActor*, const al::ActorInitInfo&,
                                                      const char*);
void startEventFlow(al::EventFlowExecutor*, const char*);
bool updateEventFlow(al::EventFlowExecutor*);
bool isActiveEventDemo(const al::LiveActor*);
bool isEqualEventDemoStartActor(const al::LiveActor*);
bool isExistTrafficAreaDirector(const al::LiveActor*);
void stopTrafficRailByTraffic(const al::LiveActor*);
void restartTrafficRailByTraffic(const al::LiveActor*);
bool tryPermitEnterTrafficNpcAndSyncDrawClipping(al::LiveActor*);
bool isDefinedEventCamera(const al::EventFlowExecutor*, const char*);
void initEventCameraObject(al::EventFlowExecutor*, const al::ActorInitInfo&, const char*);
void initEventQueryJudge(al::EventFlowExecutor*, const al::IEventFlowQueryJudge*);
void initEventCameraObjectAfterKeepPose(al::EventFlowExecutor*, const al::ActorInitInfo&,
                                        const char*);
void initEventMessageTagDataHolder(al::EventFlowExecutor*, const al::MessageTagDataHolder*);
void initEventActionNameConverter(al::EventFlowExecutor*, const TalkNpcActionAnimInfo*);
void makeEventCharacterName(sead::WBufferedSafeString*, const al::ActorInitInfo&, const char*);
void swapEventCharacterName(al::EventFlowExecutor*, const sead::WBufferedSafeString*);
void resetEventCharacterName(al::EventFlowExecutor*);
void setEventBalloonFilterOnlyMiniGame(const al::LiveActor*);
void resetEventBalloonFilter(const al::LiveActor*);
bool tryStartEventCutSceneDemo(al::LiveActor*);
void endEventCutSceneDemoOrTryEndEventCutSceneDemoBySkip(al::LiveActor*);
void tryHideDemoPlayerIfRequested(al::LiveActor*, al::EventFlowExecutor*);
void tryShowDemoPlayerIfRequested(al::LiveActor*, al::EventFlowExecutor*);
void tryStartDemoPlayerActionIfRequested(al::LiveActor*, al::EventFlowExecutor*);
bool isPlayingTextPaneAnimEventTalkMessage(const al::LiveActor*);
bool isSuccessNpcEventBalloonMessage(const al::LiveActor*);
bool isPlayingNpcEventBalloonMessageVoice(const al::LiveActor*);
bool isCloseNpcDemoEventTalkMessage(const al::LiveActor*);
void requestSwitchTalkNpcEventVolleyBall(al::LiveActor*, s32);
void skipEventDemo(al::EventFlowExecutor*);
const char* tryGetTalkNpcVolleyBallEntryName(const al::LiveActor*);
const char* tryGetTalkNpcJumpingRopeEntryName(const al::LiveActor*);
const char* tryGetTalkNpcRadiconEntryName(const al::LiveActor*);
void startWorldTravelingPeach(const al::LiveActor*);
void tryInitItemKeeperByEvent(al::LiveActor*, const al::ActorInitInfo&,
                              const al::EventFlowExecutor*);
}  // namespace rs
