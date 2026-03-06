#pragma once

#include "Library/LiveActor/LiveActor.h"

namespace al {
class EventFlowExecutor;
}  // namespace al

class SphinxRideMiniGameNpc : public al::LiveActor {
public:
    SphinxRideMiniGameNpc(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void control() override;

    void exeTalkWait();
    void exeTalkEnd();
    bool isNerveTalkEnd() const;
    void hideModel();
    void showModelAndAddDemo();

private:
    al::LiveActor* mDisplayModel = nullptr;
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
};

static_assert(sizeof(SphinxRideMiniGameNpc) == 0x118);
