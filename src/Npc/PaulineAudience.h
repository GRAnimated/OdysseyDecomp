#pragma once

#include <basis/seadTypes.h>

#include "Library/LiveActor/LiveActor.h"

class BgmAnimeSynchronizer;
class CityManRhythmInfo;
class NpcStateReaction;

class PaulineAudience : public al::LiveActor {
public:
    PaulineAudience(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void control() override;
    void endClipped() override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* self, al::HitSensor* other) override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;

    void forceControlForDance();
    void controlForDance();
    void exeDance();
    void exeSabi();
    void exeReaction();

    NpcStateReaction* mNpcStateReaction = nullptr;
    CityManRhythmInfo* mRhythmInfo = nullptr;
    u8 mInitialAnimIndex = 0;
    BgmAnimeSynchronizer* mBgmAnimeSynchronizer = nullptr;
    f32 mSabiFrame = -1.0f;
    bool _12c = false;
};

static_assert(sizeof(PaulineAudience) == 0x130);
