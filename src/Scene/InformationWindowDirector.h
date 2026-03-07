#pragma once

#include <basis/seadTypes.h>

#include "Library/Scene/ISceneObj.h"

namespace al {
class AreaObjDirector;
class LayoutInitInfo;
class PlayerHolder;
class SceneObjHolder;
struct ActorInitInfo;
}  // namespace al

class InformationWindowDirector : public al::ISceneObj {
public:
    InformationWindowDirector(al::AreaObjDirector*, al::SceneObjHolder*, const al::PlayerHolder*);
    void init(const al::ActorInitInfo&, const al::LayoutInitInfo&);

private:
    u8 _padding[0x80];
};

static_assert(sizeof(InformationWindowDirector) == 0x88);
