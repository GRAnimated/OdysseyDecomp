#pragma once

#include <basis/seadTypes.h>

#include "Library/Scene/ISceneObj.h"

class TalkNpcParam;

namespace al {
class LiveActor;
}

class TalkNpcParamHolder : public al::ISceneObj {
public:
    TalkNpcParamHolder();

    TalkNpcParam* findOrCreateParam(const al::LiveActor* actor, const char* suffix);
    TalkNpcParam* tryFindParamLocal(const al::LiveActor* actor, const char* suffix) const;
    const char* getSceneObjName() const override;

private:
    s32 mParamCount = 0;
    TalkNpcParam** mParams = nullptr;
};

static_assert(sizeof(TalkNpcParamHolder) == 0x18);
