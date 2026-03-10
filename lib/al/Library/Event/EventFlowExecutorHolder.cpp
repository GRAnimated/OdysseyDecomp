#include "Library/Event/EventFlowExecutorHolder.h"

#include "Library/Event/EventFlowExecutor.h"

namespace al {

EventFlowExecutorHolder::EventFlowExecutorHolder(s32 maxExecutors) : mMaxExecutors(maxExecutors) {
    mExecutors = new EventFlowExecutor*[maxExecutors];
    for (s32 i = 0; i < mMaxExecutors; i++)
        mExecutors[i] = nullptr;
}

void EventFlowExecutorHolder::registerExecutor(EventFlowExecutor* executor) {
    mExecutors[mExecutorCount] = executor;
    mExecutorCount++;
}

void EventFlowExecutorHolder::initAfterPlacement() {
    for (s32 i = 0; i < mExecutorCount; i++)
        mExecutors[i]->initAfterPlacement();
}

}  // namespace al
