#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

class PlayerSeparateCapFlag {
public:
    PlayerSeparateCapFlag();

    void setSeparateCap(bool isSeparateCap) { separateCap = isSeparateCap; }

    void setSeparateCapLocal(bool isSeparateCapLocal) { separateCapLocal = isSeparateCapLocal; }

    void setPuppetable(bool isPuppetable) { puppetable = isPuppetable; }

    bool isSeparateCap() const { return separateCap; }

    bool isSeparateCapLocal() const { return separateCapLocal; }

    bool isPuppetable() const { return puppetable; }

    bool isSeparate() const { return separateCap; }

    bool isPress() const { return puppetable; }

    void setPress(bool isPress) { puppetable = isPress; }

    u32 getRawFlags() const { return mRawFlags; }

    sead::Vector3f* getSeparateCapLocalOffset() { return &mSeparateCapLocalOffset; }

    const sead::Vector3f& getSeparateCapLocalOffset() const { return mSeparateCapLocalOffset; }

private:
    union {
        struct {
            bool separateCap;
            bool separateCapLocal;
            bool puppetable;
            u8 padding;
        };

        u32 mRawFlags;
    };

    sead::Vector3f mSeparateCapLocalOffset;
};

static_assert(sizeof(PlayerSeparateCapFlag) == 0x10);
