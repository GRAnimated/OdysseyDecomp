#pragma once

namespace al {
class AreaObjDirector;
class PlayerHolder;
class SwitchAreaGroup;
}  // namespace al

class SwitchOnPlayerOnGroundAreaGroup {
public:
    static al::SwitchAreaGroup* tryCreate(const char*, al::AreaObjDirector*,
                                          const al::PlayerHolder*);
};
