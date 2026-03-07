#pragma once

#include "Library/LiveActor/LiveActor.h"

class OpeningStageStartDemo : public al::LiveActor {
public:
    virtual bool isEnableStart() const;
    void startDemo();
};
