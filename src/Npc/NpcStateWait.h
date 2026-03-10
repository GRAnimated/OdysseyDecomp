#pragma once

#include "Library/Nerve/NerveStateBase.h"

class NpcStateWaitParam;
class NpcStateTurnParam;
class NpcStateRumbleParam;

namespace al {
class LiveActor;
class RumbleCalculator;
struct ActorInitInfo;
}  // namespace al

class NpcStateWait : public al::ActorStateBase {
public:
    NpcStateWait(al::LiveActor*, const al::ActorInitInfo&, const NpcStateWaitParam*,
                 const NpcStateTurnParam*, const NpcStateRumbleParam*);

    void appear() override;
    void control() override;

    void setWaitAfter();
    void setWait();
    void startWait();
    void invalidateTurn();
    void startTurnEnd();
    bool tryStartTurn(const NpcStateTurnParam* param);

    void exeWait();
    void exeWaitAfter();
    void exeTurn();
    void exeTurnEnd();
    void exeTrampled();

private:
    const NpcStateWaitParam* mWaitParam = nullptr;
    const NpcStateTurnParam* mTurnParam = nullptr;
    const NpcStateRumbleParam* mRumbleParam = nullptr;
    al::RumbleCalculator* mRumbleCalculator = nullptr;
    s32 mRumbleTimer = -1;
    bool mIsWaitAfter = false;
    bool mIsTurnInvalid = false;
};

static_assert(sizeof(NpcStateWait) == 0x48);
