#pragma once

#include <prim/seadSafeString.h>

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/LiveActor/LiveActor.h"

class SaveObjInfo;

namespace al {
class AreaObjGroup;
class EventFlowEventData;
class EventFlowExecutor;
class KeyPoseKeeper;
}  // namespace al

class SphinxQuiz : public al::LiveActor, public al::IEventFlowEventReceiver {
public:
    SphinxQuiz(const char* name);

    void init(const al::ActorInitInfo& info) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* self, al::HitSensor* other) override;
    bool receiveEvent(const al::EventFlowEventData* event) override;

    void exeWait();
    void exeKeyWait();
    void exeKeyMove();
    void exeWaitMiniGame();

private:
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    const char* mQuizType = "SphinxQuiz";
    SaveObjInfo* mKeyMoveSaveObjInfo = nullptr;
    SaveObjInfo* mSaveObjInfo = nullptr;
    sead::WFixedSafeString<64> mEventCharacterName;
    al::AreaObjGroup* mRouteKillAreaGroup = nullptr;
    al::KeyPoseKeeper* mKeyPoseKeeper = nullptr;
    s32 mKeyWaitTime = 0;
    s32 mKeyMoveTime = 0;
    s32 mRecordCounter = -1;
    s32 mShineIndex = -1;
    bool mHasFailed = false;
};

static_assert(sizeof(SphinxQuiz) == 0x1F0);
