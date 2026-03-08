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
        PlayerCreatorFunction creationFunc = nullptr;
        getEntryIndex(&creationFunc, className);
        if (!creationFunc)
            return nullptr;
        const char* displayName = nullptr;
        al::getDisplayName(&displayName, initInfo);
        return creationFunc(displayName);
    }
};
