#pragma once

#include "Library/LiveActor/LiveActor.h"

namespace al {
struct ActorInitInfo;
class CollisionPartsConnector;

class FootPrint : public LiveActor {
public:
    FootPrint(const ActorInitInfo& initInfo, const char* name);
    void startDisappear();
    bool isDisappear() const;

private:
    CollisionPartsConnector* mConnector;
    const char* mMaterialName;
};

static_assert(sizeof(FootPrint) == 0x118);
}  // namespace al
