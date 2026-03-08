#pragma once

#include <prim/seadSafeString.h>

#include "Library/Scene/ISceneObj.h"

#include "Scene/SceneObjFactory.h"

namespace al {
class IUseSceneObjHolder;
}

class RankingNameHolder : public al::ISceneObj {
public:
    static constexpr s32 sSceneObjId = SceneObjID_RankingNameHolder;

    RankingNameHolder();
    ~RankingNameHolder() override = default;

    void write();

private:
    sead::FixedSafeString<128> mName;
};

namespace rs {
void createRankingNameHolder(const al::IUseSceneObjHolder* user);
RankingNameHolder* tryGetRankingNameHolder(const al::IUseSceneObjHolder* user);
}  // namespace rs
