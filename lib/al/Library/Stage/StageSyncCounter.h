#pragma once

#include <basis/seadTypes.h>

#include "Library/Execute/IUseExecutor.h"
#include "Library/Scene/ISceneObj.h"

namespace al {
class StageSyncCounter : public al::IUseExecutor, public al::ISceneObj {
public:
    StageSyncCounter();
    virtual void execute() override;
    virtual const char* getSceneObjName() const override;
    virtual void initAfterPlacementSceneObj(const al::ActorInitInfo&) override;

    s32 getCounter() const { return mCounter; }

private:
    s32 mCounter = 0;
};
}  // namespace al