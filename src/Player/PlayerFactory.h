#pragma once

#include "Library/Factory/Factory.h"
#include "Library/Placement/PlacementFunction.h"

namespace al {
class LiveActor;
struct ActorInitInfo;
}  // namespace al

using PlayerCreatorFunction = al::LiveActor* (*)(const char* actorName);

class PlayerFactory : public al::Factory<PlayerCreatorFunction> {
public:
    PlayerFactory();
    al::LiveActor* createActor(const al::ActorInitInfo& initInfo, const char* className) {
        PlayerCreatorFunction creator;
        if (getEntryIndex(&creator, className) < 0)
            return nullptr;
        const char* objectName = nullptr;
        al::getObjectName(&objectName, initInfo);
        return creator(objectName);
    }
};
