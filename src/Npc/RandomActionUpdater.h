#pragma once

#include "Library/Nerve/NerveExecutor.h"

namespace al {
class LiveActor;
}  // namespace al

class TalkNpcActionAnimInfo;

class RandomActionUpdater : public al::NerveExecutor {
public:
    RandomActionUpdater(al::LiveActor*, const TalkNpcActionAnimInfo*);
    void initBalloonAction(const char* actionName);
    void update();
    void forceEnd();
    void restart();
    void tryStartWaitActionIfNotPlaying();
    void startActionOneTime(const char*);
    bool isPlayingBalloonAction() const;

private:
    u8 _10[0x98];
};

static_assert(sizeof(RandomActionUpdater) == 0xA8);
