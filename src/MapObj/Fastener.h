#pragma once

#include "Library/LiveActor/LiveActor.h"

class Fastener : public al::LiveActor {
public:
    Fastener(const char* name);

    void tryUpdateDisplayModel();
};
