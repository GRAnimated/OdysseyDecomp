#pragma once

#include "Library/Scene/ISceneObj.h"

namespace al {

class DataStoreDirector : public ISceneObj {
public:
    const char* getSceneObjName() const override;
    void initAfterPlacementSceneObj(const ActorInitInfo&) override {}
    void initSceneObj() override {}
};

class RankingDirector : public ISceneObj {
public:
    const char* getSceneObjName() const override;
    void initAfterPlacementSceneObj(const ActorInitInfo&) override {}
    void initSceneObj() override {}
};

class NetworkSystem {
public:
    DataStoreDirector* getDataStoreDirector() const;
    RankingDirector* getRankingDirector() const;
};
}  // namespace al
