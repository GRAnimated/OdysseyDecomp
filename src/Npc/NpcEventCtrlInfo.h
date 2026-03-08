#pragma once

#include <basis/seadTypes.h>

#include "Library/Scene/ISceneObj.h"

#include "Npc/NpcEventBalloonInfo.h"

namespace al {
class BalloonOrderGroupHolder;
class EventFlowChoiceInfo;
class EventFlowExecutorHolder;
class EventFlowNode;
class LiveActor;
class SceneEventFlowMsg;
}  // namespace al

class CostumePatternChecker;
class EventFlowSceneExecuteCtrl;
class NpcEventSceneConstData;
class NpcEventSceneInfo;

struct BalloonFilterInfo {
    BalloonFilterInfo() : actor(nullptr), type(-1) {}
    const al::LiveActor* actor;
    s32 type;
};

class NpcEventCtrlInfo : public al::ISceneObj {
public:
    NpcEventCtrlInfo(const NpcEventSceneInfo& sceneInfo,
                     const NpcEventSceneConstData& sceneConstData,
                     EventFlowSceneExecuteCtrl* execCtrl);

    bool isCloseTalk() const;
    void popBalloonInfo(NpcEventBalloonInfo* info);
    void popTalkInfo(NpcEventTalkInfo* dest);
    void requestShowBalloonMessage(const NpcEventBalloonInfo& info);
    void requestShowTalkMessage(const al::EventFlowNode* node,
                                const NpcEventTalkInfo& src);
    void requestCloseTalkMessage(const al::LiveActor* actor);
    void requestCloseWipeFadeBlack(al::EventFlowNode* node, s32 frames);
    void requestOpenWipeFadeBlack(al::EventFlowNode* node, s32 frames);
    void setBalloonFilterOnlyMiniGame(const al::LiveActor* actor);
    void resetBalloonFilter(const al::LiveActor* actor);
    void startChoice(const al::EventFlowNode* node,
                     al::EventFlowChoiceInfo* choiceInfo);
    s32 getChoiceMessageNum() const;
    const char16* getChoiceMessage(s32 index) const;
    s32 getChoiceCancelIndex() const;
    const char16* tryGetChoiceTalkMessage() const;
    void endChoice(s32 selectedChoice);
    bool isEnableCancelChoice() const;
    const char* getSceneObjName() const override;
    ~NpcEventCtrlInfo() override;

private:
    friend class NpcEventDirector;

    const NpcEventSceneInfo* mSceneInfo;
    const NpcEventSceneConstData* mSceneConstData;
    al::SceneEventFlowMsg* mSceneEventFlowMsg = nullptr;
    EventFlowSceneExecuteCtrl* mExecCtrl;
    al::EventFlowExecutorHolder* mExecutorHolder = nullptr;
    al::BalloonOrderGroupHolder* mBalloonGroupHolder = nullptr;
    NpcEventBalloonInfo mBalloonInfo;
    BalloonFilterInfo* mBalloonFilter;
    NpcEventTalkInfo mTalkInfo;
    al::EventFlowChoiceInfo* mChoiceInfo = nullptr;
    CostumePatternChecker* mCostumePatternChecker = nullptr;
    bool mIsCloseWipeRequested = false;
    bool mIsOpenWipeRequested = false;
    s32 mCloseWipeFrames = -1;
    s32 mOpenWipeFrames = -1;
};

static_assert(sizeof(NpcEventCtrlInfo) == 0xD8);
