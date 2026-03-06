#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.h"

class SphinxRide;
struct SphinxRideParam;

class SphinxRideStateStop : public al::HostStateBase<SphinxRide> {
public:
    SphinxRideStateStop(SphinxRide* host, const SphinxRideParam& param);
    void appear() override;
    void kill() override;
    void exeStopStart();
    void exeStopButton();
    bool updateStopCommon();
    void exeStopButtonSwing();
    void updateStopStrong();
    void exeStopSwing();

private:
    const SphinxRideParam* mParam;
    bool mIsSwingStop = false;
    s32 mButtonFrame = 0;
    s32 mSwingFrame = 0;
    bool mIsGetOff = false;
};

static_assert(sizeof(SphinxRideStateStop) == 0x38);
