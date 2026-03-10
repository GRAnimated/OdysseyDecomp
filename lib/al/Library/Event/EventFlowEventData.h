#pragma once

namespace al {
class EventFlowEventData {
public:
    const char* getEventName() const { return mName; }

    const char* mName;
};
}  // namespace al
