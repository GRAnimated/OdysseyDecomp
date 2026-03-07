#pragma once

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
};
