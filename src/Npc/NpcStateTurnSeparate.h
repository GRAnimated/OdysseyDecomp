#pragma once

#include <math/seadVector.h>

#include "Npc/NpcStateTurnBase.h"

struct TurnDirInfo;

class NpcStateTurnSeparate : public NpcStateTurnBase {
public:
    NpcStateTurnSeparate(const char* name, al::LiveActor* actor);

    void startTurnToTarget(const sead::Vector3f& target);
    bool tryStartTurnToTarget(const sead::Vector3f& target, f32 threshold);
    void startTurnToDir(const sead::Vector3f& dir);
    bool tryStartTurnToDir(const sead::Vector3f& dir, f32 threshold);
    void exeTurn();

private:
    const TurnDirInfo* mTurnDirInfo = nullptr;
    sead::Vector3f mFrontDir = {0, 0, 0};
    sead::Vector3f mUpDir = {0, 0, 0};
    f32 mAngle = 0;
};

static_assert(sizeof(NpcStateTurnSeparate) == 0x50);
