#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
}
class PlayerAreaChecker;
class PlayerEffect;

class PlayerWetControl {
public:
    PlayerWetControl(const al::LiveActor* player, al::LiveActor* model,
                     const PlayerAreaChecker* areaChecker);

    bool isWet() const;
    void reset();
    void updateModelRoughness(f32 roughness);
    void recordInWater();
    void recordWet();
    void recordWaterSurface();
    void recordHeavyLandPuddle();
    void recordPuddleRolling();
    void recordForestWaterFall();
    void recordWaterSplash();
    void recordWetBySensor();
    void update();

    void setEffect(PlayerEffect* effect) { mEffect = effect; }

private:
    const al::LiveActor* mPlayer;
    PlayerEffect* mEffect;
    al::LiveActor* mModel;
    const PlayerAreaChecker* mAreaChecker;
    bool mIsInWetArea;
    u8 mPadding21[3];
    s32 mInWaterCounter;
    s32 mWetCounter;
    f32 mRoughness;
    bool mIsWaterSurface;
    u8 mPadding31[7];
};

static_assert(sizeof(PlayerWetControl) == 0x38);
