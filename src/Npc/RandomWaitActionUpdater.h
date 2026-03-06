#pragma once

namespace al {
struct ActorInitInfo;
class LiveActor;
}  // namespace al

class TalkNpcParam;

class RandomWaitActionUpdater {
public:
    RandomWaitActionUpdater(al::LiveActor* actor, const al::ActorInitInfo& info,
                            const TalkNpcParam* param, const char* waitAction,
                            const char* byeAction);
    void update();
    void setDisableBalloonAction();

private:
    u8 _0[0x10];
};

static_assert(sizeof(RandomWaitActionUpdater) == 0x10);
