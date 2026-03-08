#pragma once

#include <basis/seadTypes.h>

namespace al {
class LayoutActor;

class LayoutActorGroup {
public:
    LayoutActorGroup(const char* name, s32 maxCount);
    void registerActor(LayoutActor* actor);
};
}  // namespace al
