#pragma once

#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

namespace al {
class ByamlIter;
}

class GhostPlayer : public al::LiveActor {
public:
    GhostPlayer(const char* name, s32 goalIndex, s32 param);

    void initGhostPlayDataFromByaml(al::ByamlIter* iter);
    void initWithArchiveName(const al::ActorInitInfo& initInfo, const char* archiveName,
                             const char* suffix);
    void initThrowCap(al::LiveActor* cap);
    bool isPlayDone() const;
    void start();
    void attachJumpToTarget(const sead::Vector3f& pos);

    s32 mGoalIndex;
    s32 _10c;
    s32 mRaceManStep;
    u8 _114[0x19d - 0x114];
    bool _19d;
    bool _19e;
    u8 _19f;
    sead::Vector3f _1a0;
    u8 _1ac[0x230 - 0x1ac];
};

static_assert(sizeof(GhostPlayer) == 0x230);
