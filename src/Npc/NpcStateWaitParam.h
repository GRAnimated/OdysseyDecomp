#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

class NpcStateWaitParam {
public:
    NpcStateWaitParam(const char* waitAction, const char* waitAfterAction, const char* turnAction,
                      const char* _18, const char* _20, const char* trampledAction, bool _30,
                      const sead::Vector3f* vec, bool _40);

    const char* mWaitAction;
    const char* mWaitAfterAction;
    const char* mTurnAction;
    const char* _18;
    const char* _20;
    const char* mTrampledAction;
    bool _30;
    sead::Vector3f _34 = sead::Vector3f::zero;
    bool _40;
};

static_assert(sizeof(NpcStateWaitParam) == 0x48);
