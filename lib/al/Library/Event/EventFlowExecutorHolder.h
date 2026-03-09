#pragma once

#include <basis/seadTypes.h>

namespace al {
class EventFlowExecutor;

class EventFlowExecutorHolder {
public:
    EventFlowExecutorHolder(s32 maxExecutors);

    void registerExecutor(EventFlowExecutor* executor);
    void initAfterPlacement();

private:
    s32 mMaxExecutors;
    s32 mExecutorCount = 0;
    EventFlowExecutor** mExecutors = nullptr;
};

}  // namespace al
