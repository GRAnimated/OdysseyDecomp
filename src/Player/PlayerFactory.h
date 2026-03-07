#pragma once

#include "Library/Factory/Factory.h"

namespace al {
class LiveActor;
struct ActorInitInfo;
}  // namespace al

using PlayerCreatorFunction = al::LiveActor* (*)(const char* actorName);

class PlayerFactory : public al::Factory<PlayerCreatorFunction> {
public:
    PlayerFactory();
    al::LiveActor* createActor(const al::ActorInitInfo&, const char*);
};
