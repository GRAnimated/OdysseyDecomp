#pragma once

#include <basis/seadTypes.h>

#include "Library/Scene/ItemDirectorBase.h"

class GameDataHolder;

namespace al {
struct ActorInitInfo;
}  // namespace al

class ProjectItemDirector : public al::ItemDirectorBase {
public:
    ProjectItemDirector();
    void appearItem(const char*, const sead::Vector3f&, const sead::Quatf&,
                    const al::HitSensor*) override;
    void acquireItem(const al::LiveActor*, al::HitSensor*, const char*) override;
    void declareItem(const char*, const al::ActorInitInfo&) override;
    void endInit() override;
    void initPlayerDeadCoin(GameDataHolder* holder, const al::ActorInitInfo& initInfo);

private:
    u8 _padding[0x60];
};

static_assert(sizeof(ProjectItemDirector) == 0x68);
