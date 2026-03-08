#pragma once

#include <basis/seadTypes.h>

namespace sead {
template <typename T>
class Matrix34;
using Matrix34f = Matrix34<f32>;
}  // namespace sead

namespace al {
class AddDemoInfo;
struct ActorInitInfo;
class LiveActor;
class Scene;

class DemoActorHolder {
public:
    void startSequence();
    void updateSequence();
    void updateGraphics();
    void kill();
    bool isEndSequence() const;
    s32 getCurrentDemoFrame() const;
    s32 getCurrentDemoFrameMax() const;
};

AddDemoInfo* registDemoRequesterToAddDemoInfo(const LiveActor* actor, const ActorInitInfo& initInfo,
                                              s32 index);
void registActorToDemoInfo(LiveActor* actor, const ActorInitInfo& initInfo);
void addDemoActorFromAddDemoInfo(const LiveActor* actor, const AddDemoInfo* info);
void addDemoActorFromDemoActorHolder(const LiveActor* actor, const DemoActorHolder* holder);
void addDemoActorFromDemoActorHolder(const Scene* scene, const DemoActorHolder* holder);
void setDemoInfoDemoName(const LiveActor* actor, const char* name);
void killForceBeforeDemo(LiveActor* actor);
void prepareSkip(LiveActor* actor, s32);
void invalidateLODWithSubActor(LiveActor*);
bool isActiveDemo(const Scene* scene);

namespace alDemoFunction {
DemoActorHolder* createDemoActorHolder(const char* name, const ActorInitInfo& initInfo,
                                       const sead::Matrix34f* mtx, s32 count, bool);
}  // namespace alDemoFunction

}  // namespace al

namespace alDemoUtilTmp {
bool isActiveDemo(const al::LiveActor*);
}
