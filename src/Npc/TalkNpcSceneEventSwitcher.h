#pragma once

#include <basis/seadTypes.h>

#include "Library/Scene/ISceneObj.h"

class TalkNpcSceneEventSwitcher : public al::ISceneObj {
public:
    TalkNpcSceneEventSwitcher();

    const char* getSceneObjName() const override;

    s32 _8 = 0;
    void* _10 = nullptr;
    void* _18 = nullptr;
    void* _20 = nullptr;
};

static_assert(sizeof(TalkNpcSceneEventSwitcher) == 0x28);
