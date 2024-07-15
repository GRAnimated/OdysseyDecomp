#pragma once

namespace al {
class ActorInitInfo;
class LiveActor;
}  // namespace al

class AppearSwitchSave {
public:
    AppearSwitchSave(al::LiveActor*, const al::ActorInitInfo&);
    void onSwitch();
    void onSwitchDemo();
    bool isOn() const;

private:
    void* filler[10];
};
