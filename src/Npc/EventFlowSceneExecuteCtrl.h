#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveExecutor.h"

class EventFlowSceneExecuteCtrl : public al::NerveExecutor {
public:
    EventFlowSceneExecuteCtrl();

private:
    u8 _padding[0x10];
};

static_assert(sizeof(EventFlowSceneExecuteCtrl) == 0x20);
