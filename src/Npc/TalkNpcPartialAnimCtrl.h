#pragma once

#include <math/seadVector.h>

#include "Library/Nerve/NerveExecutor.h"

namespace al {
class LiveActor;
}

class NpcStateReactionParam;
class TalkNpcActionAnimInfo;

class TalkNpcPartialAnimCtrl : public al::NerveExecutor {
public:
    TalkNpcPartialAnimCtrl(al::LiveActor*, const TalkNpcActionAnimInfo*);
    ~TalkNpcPartialAnimCtrl() override = default;

    bool isReactionCap() const;
    void update();
    void forceEndAndInvalidateByeBye();
    void forceEndAndInvalidateReaction();
    void startReaction();
    void startReactionCap();

    void exeNone();
    void exeReaction();
    void endReaction();
    void exeReactionCap();
    void endReactionCap();
    void exeByeBye();

    al::LiveActor* mActor;
    const TalkNpcActionAnimInfo* mAnimInfo;
    bool _20 = false;
    bool _21 = false;
    NpcStateReactionParam* mReactionParam = nullptr;
    const char* mByeByeBaseJointName = nullptr;
    sead::Vector3f mByeByeLocalAxisFront = {0.0f, 0.0f, 0.0f};
    s32 _44 = -1;
};

static_assert(sizeof(TalkNpcPartialAnimCtrl) == 0x48);
