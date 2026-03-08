#pragma once

#include <container/seadPtrArray.h>
#include <math/seadQuat.h>

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Scene/ISceneObj.h"

namespace al {
class AreaObjGroup;
class CameraTicket;
class EventFlowExecutor;
class ParabolicPath;
}  // namespace al

class GhostPlayer;
class RaceManGoal;
class TalkNpcCap;
struct RaceRankAreaInfo;

class RaceManRace : public al::LiveActor,
                    public al::IEventFlowEventReceiver,
                    public al::ISceneObj {
public:
    RaceManRace(const char* name);

    void init(const al::ActorInitInfo& initInfo) override;
    void control() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                    al::HitSensor* self) override;

    bool receiveEvent(const al::EventFlowEventData* data) override;

    const char* getSceneObjName() const override;

    void startCountDown();
    void startRace();
    void trySetRaceManStep(s32 step);
    void checkGoalRaceMan();
    s32 calcRank() const;
    bool isGoalNpc(sead::Vector3f* pos) const;
    void onGoalPlayer(bool isWin);
    void onGoalNpc(const sead::Vector3f& pos);
    void exeGoalNpcLoseWait();
    void exeGoalNpcWinPlayerGoal();
    void exeGoalNpcWin();
    void startRetry();
    void exeStartRetry();
    void exeEventRetry();
    void exeWaitReEnterStage();
    bool isWaitReEnterStage() const;
    bool isWaitEndStage() const;
    void attachDemoPlayerToGoal();
    void exeAttachJumpPlayer();
    void exeAttachJumpPlayerEnd();
    void exeResultTurn();
    void exeResult();
    void exeStartWait();
    void exeRace();
    void exeWaitEndStage();

private:
    sead::FixedPtrArray<GhostPlayer, 4> mGhostPlayers;
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    RaceManGoal* mRaceManGoal = nullptr;
    al::CameraTicket* mCameraTicket = nullptr;
    al::ParabolicPath* mParabolicPath = nullptr;
    sead::Quatf mStartQuat = sead::Quatf::unit;
    sead::Quatf mGoalQuat = sead::Quatf::unit;
    al::AreaObjGroup* mAreaObjGroup = nullptr;
    sead::PtrArray<RaceRankAreaInfo> mRankAreas;
    TalkNpcCap* mTalkNpcCap = nullptr;
    bool mIsGoalPlayer = false;
    bool mIsWin = false;
};

static_assert(sizeof(RaceManRace) == 0x1B0);
