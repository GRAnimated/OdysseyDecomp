#pragma once

#include <basis/seadTypes.h>

#include "Library/Audio/IUseAudioKeeper.h"
#include "Library/Scene/ISceneObj.h"

class BgmAnimeSyncDirector : public al::ISceneObj, public al::IUseAudioKeeper {
public:
    BgmAnimeSyncDirector();
    const char* getSceneObjName() const override;
    al::AudioKeeper* getAudioKeeper() const override;
    void initAfterPlacementSceneObj(const al::ActorInitInfo&) override {}
    void initSceneObj() override {}

private:
    u8 _padding[0x10];
};

static_assert(sizeof(BgmAnimeSyncDirector) == 0x20);
