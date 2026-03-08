#pragma once

#include "Library/LiveActor/LiveActor.h"

class OpeningStageStartDemo : public al::LiveActor {
public:
    virtual void startDemo();
    virtual void endDemo();
    virtual bool isEnableStart() const;
};
