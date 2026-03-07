#pragma once

namespace al {
class AreaObjDirector;
class LayoutInitInfo;
class PlayerHolder;
}  // namespace al

class GameDataHolder;

class LocationNameCtrl {
public:
    LocationNameCtrl(al::AreaObjDirector*, GameDataHolder*, const al::LayoutInitInfo&,
                     const al::PlayerHolder*);
};
