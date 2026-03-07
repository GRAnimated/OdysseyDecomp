#pragma once

namespace al {
class AreaObjDirector;
class SwitchAreaGroup;
}  // namespace al

class SwitchKeepOnIgnoreOffAreaTarget {
public:
    static al::SwitchAreaGroup* tryCreate(const char*, al::AreaObjDirector*);
};
