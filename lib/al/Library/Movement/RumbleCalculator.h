#pragma once

#include <math/seadVector.h>

namespace al {

class RumbleCalculator {
public:
    RumbleCalculator(f32 frequency, f32 phaseOffset, f32 amplitude, u32 duration);

    void setParam(f32 frequency, f32 phaseOffset, f32 amplitude, u32 duration);
    void start(u32 startFrame);
    void calc();
    void reset();

    virtual void calcValues(sead::Vector3f* result, const sead::Vector3f& input) = 0;

    u32 mFrame;
    u32 mDuration;
    sead::Vector3f mResult;
    f32 mFrequency;
    f32 mPhaseOffset;
    f32 mAmplitude;
};

class RumbleCalculatorCosMultLinear : public RumbleCalculator {
public:
    RumbleCalculatorCosMultLinear(f32 frequency, f32 phaseOffset, f32 amplitude, u32 duration);

    void calcValues(sead::Vector3f* result, const sead::Vector3f& input) override;
};

class RumbleCalculatorCosAddOneMultLinear : public RumbleCalculator {
public:
    RumbleCalculatorCosAddOneMultLinear(f32 frequency, f32 phaseOffset, f32 amplitude, u32 duration);

    void calcValues(sead::Vector3f* result, const sead::Vector3f& input) override;
};

}  // namespace al
