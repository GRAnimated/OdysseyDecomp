#pragma once

#include "Library/LiveActor/LiveActor.h"

class MayorItem : public al::LiveActor {
public:
    MayorItem(const char* name);

    void acceptPresent();
    void endPresent();
    void collectPresent();
    bool isHold() const;

private:
    u8 _108[0xC8];
};

static_assert(sizeof(MayorItem) == 0x1D0);
