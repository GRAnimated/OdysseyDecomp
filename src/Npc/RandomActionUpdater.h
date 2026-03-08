#pragma once

#include <prim/seadSafeString.h>

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

    void exeWait();
    void exeBalloonAction();
    void exePlayOneTime();
    void exeRandomOneTime();
    void exeRandomLoop();

    al::LiveActor* mActor;
    const TalkNpcActionAnimInfo* mActionAnimInfo;
    const char* _20;
    const char* _28;
    const char* _30;
    sead::FixedSafeString<64> mBalloonActionName;
    const char* _90;
    bool _98;
    bool _99;
    s32 _9c;
    f32 _a0;
};

static_assert(sizeof(RandomActionUpdater) == 0xA8);
