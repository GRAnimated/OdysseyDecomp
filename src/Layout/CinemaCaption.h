#pragma once

#include <basis/seadTypes.h>

namespace al {
class LayoutInitInfo;
}

class CinemaCaption {
public:
    CinemaCaption(const al::LayoutInitInfo&);

private:
    u8 _padding[0x148];
};

static_assert(sizeof(CinemaCaption) == 0x148);
