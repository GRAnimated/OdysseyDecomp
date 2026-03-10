#pragma once

#include <basis/seadTypes.h>

namespace sead {
template <typename T>
class Matrix34;
using Matrix34f = Matrix34<f32>;
}  // namespace sead

namespace al {
struct ActorInitInfo;

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

namespace alDemoFunction {
DemoActorHolder* createDemoActorHolder(const char* name, const ActorInitInfo& initInfo,
                                       const sead::Matrix34f* mtx, s32 count, bool);
}  // namespace alDemoFunction

}  // namespace al
