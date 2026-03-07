#pragma once

#include "Library/LiveActor/LiveActor.h"

class Shibaken : public al::LiveActor {
public:
    Shibaken(const char* name);

    void init(const al::ActorInitInfo& info) override;
};
