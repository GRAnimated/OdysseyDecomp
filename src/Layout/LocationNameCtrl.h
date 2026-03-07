#pragma once

#include <basis/seadTypes.h>

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

private:
    u8 _padding[0x40];
};

static_assert(sizeof(LocationNameCtrl) == 0x40);
