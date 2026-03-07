#pragma once

namespace al {
class LiveActor;
struct ActorInitInfo;
}  // namespace al

class PlayerFactory {
public:
    PlayerFactory();
    al::LiveActor* createActor(const al::ActorInitInfo&, const char*);

private:
    void* mBuffer = nullptr;
    int mSize = 0;
};
