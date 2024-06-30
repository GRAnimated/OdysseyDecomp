#pragma once

#include <basis/seadTypes.h>

#include "Library/HostIO/HioNode.h"

namespace al {
class LiveActor;
class ActorInitInfo;
class EffectSystem;
class PlacementId;

class DemoDirector : public al::HioNode {
public:
    DemoDirector(s32);

    void addDemoActorWithSubActor(al::LiveActor*);
    void addDemoActor(al::LiveActor*);
    virtual void endInit(const al::ActorInitInfo&);
    void isActiveDemo() const;
    void getActiveDemoName() const;
    void requestStartDemo(const char*);
    void requestEndDemo(const char*);
    void tryAddDemoActor(al::LiveActor*);
    void getDemoActorList() const;
    void getDemoActorNum() const;
    virtual void updateDemoActor(al::EffectSystem*);
    void registDemoRequesterToAddDemoInfo(al::PlacementId const&);
    void findOrCreateAddDemoInfo(const al::PlacementId&);
    void registActorToAddDemoInfo(al::LiveActor*, const al::PlacementId&);
    void tryFindAddDemoInfo(const al::PlacementId&) const;
    void findAddDemoInfo(const al::PlacementId&) const;
    virtual void startDemo(const char*);
    virtual void endDemo(const char*);

private:
    const char* mDemoName;
    void* dword10;
    void* qword18;
    void* dword20;
    void* dword24;
    void* qword28;
    void* qword30;
    void* qword38;
    void* dword40;
    void* qword48;
};
}  // namespace al
