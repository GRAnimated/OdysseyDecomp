#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

class PlayerDemoActionFlag {
public:
    PlayerDemoActionFlag();
    void reset();

    bool isInvalidateCapOn() const { return (mFlags & 0xFF000000) != 0; }

    bool isCapOff() const { return (mFlags & 0xFF000000) != 0; }

    bool isShadowLengthFixed() const { return (mFlags & 0x00FF0000) != 0; }

    f32 getShadowLength() const { return shadowLength; }

    bool isDemoAction() const { return mIsDemoAction; }

    bool isEnableIK() const { return (mFlags & 0x000000FF) != 0; }

    bool isLookAtTargetPositionEnabled() const { return (mFlags & 0x0000FF00) != 0; }

    const sead::Vector3f& getLookAtTargetPosition() const { return lookAtTargetPosition; }

    void clearDemoAction() { mIsDemoAction = false; }

private:
    u32 mFlags;
    bool mIsDemoAction;
    u8 _5[3];

    union {
        struct {
            u64 actionData;

            union {
                u64 actionParam;

                struct {
                    u32 _10;
                    f32 shadowLength;
                };
            };
        };

        struct {
            sead::Vector3f lookAtTargetPosition;
            f32 lookAtPadding;
        };
    };
};

static_assert(sizeof(PlayerDemoActionFlag) == 0x18);
