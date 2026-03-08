#pragma once

#include <basis/seadTypes.h>

namespace al {

class EventFlowExecutorHolder {
public:
    EventFlowExecutorHolder(s32 maxExecutors);

private:
    void* _00;
    s32 _08;
};

}  // namespace al
