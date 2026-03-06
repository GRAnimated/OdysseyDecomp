#pragma once

#include "Library/Event/IUseEventFlowData.h"
#include "Library/Nerve/IUseNerve.h"

namespace al {
struct ActorInitInfo;
class EventFlowDataHolder;
class LiveActor;
class Nerve;
class NerveKeeper;

class EventFlowMovement : public IUseEventFlowData, public IUseNerve {
public:
    EventFlowMovement(const char* name, LiveActor* actor);

    EventFlowDataHolder* getEventFlowDataHolder() const override;
    virtual void init(const ActorInitInfo&);
    virtual void appear();
    virtual void kill();
    virtual void control();
    NerveKeeper* getNerveKeeper() const override;
    virtual bool isTurnMovement() const;
    virtual bool isWaitAtPointMovement() const;

    void movement();
    void initNerve(const Nerve* nerve, s32 maxStates);

protected:
    const char* mName;
    LiveActor* mActor;
    NerveKeeper* mNerveKeeper = nullptr;
    EventFlowDataHolder* mDataHolder = nullptr;
};
}  // namespace al
