#pragma once

#include <basis/seadTypes.h>

namespace al {
struct ActorInitInfo;
class LiveActor;
}  // namespace al

class RandomActionUpdater;
class TalkNpcActionAnimInfo;
class TalkNpcParam;

class RandomWaitActionUpdater {
public:
    RandomWaitActionUpdater(al::LiveActor* actor, const al::ActorInitInfo& info,
                            const TalkNpcParam* param, const char* waitAction,
                            const char* byeAction);
    void setDisableBalloonAction();
    void setRandomOutbreakProbability(f32 probability);
    void update();

private:
    TalkNpcActionAnimInfo* mActionAnimInfo = nullptr;
    RandomActionUpdater* mRandomActionUpdater = nullptr;
};

static_assert(sizeof(RandomWaitActionUpdater) == 0x10);
