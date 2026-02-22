#pragma once

#include "Library/LiveActor/LiveActor.h"

class BossRaid : public al::LiveActor {
public:
    BossRaid(const char* name);
    void init(const al::ActorInitInfo& info) override;

    void resetChainAll();
    void startActionMain(const char* actionName);
    void validateCollisionAll();
    void startElectricParts();
    void endElectricParts();
    s32 getShotLevel() const;

    u8 _108[0x18];
    al::LiveActor* mArmorActor = nullptr;

private:
};
