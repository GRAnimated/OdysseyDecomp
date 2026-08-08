#pragma once

#include <basis/seadTypes.h>

class PlayerOxygen {
public:
    PlayerOxygen();

    void setup(s32 no_reduce_frame, s32 reduce_frame, s32 recovery_frame, s32 damage_interval);
    void reset();
    void reduce();
    void recovery();
    bool isTriggerDamage() const;
    f32 getRate() const { return mOxygenLevel; }
    bool isReducing() const { return mFramesReducing > 0; }

private:
    f32 mOxygenLevel = 1.0;
    s32 mFramesReducing = 0;
    s32 mFramesWithoutOxygen = 0;
    s32 mOxygenNoReduceFrame = 0;
    s32 mOxygenReduceFrame = 0;
    s32 mOxygenRecoveryFrame = 0;
    s32 mOxygenDamageInterval = 0;
};

static_assert(sizeof(PlayerOxygen) == 0x1C);
