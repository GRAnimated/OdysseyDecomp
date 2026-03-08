#include "Npc/RankingNameHolder.h"

#include "Library/Scene/SceneObjUtil.h"

RankingNameHolder::RankingNameHolder() = default;

void RankingNameHolder::write() {}

void rs::createRankingNameHolder(const al::IUseSceneObjHolder* user) {
    if (!al::isExistSceneObj(user, RankingNameHolder::sSceneObjId)) {
        auto* holder = new RankingNameHolder();
        al::setSceneObj(user, holder, RankingNameHolder::sSceneObjId);
    }
}

RankingNameHolder* rs::tryGetRankingNameHolder(const al::IUseSceneObjHolder* user) {
    return static_cast<RankingNameHolder*>(
        al::tryGetSceneObj(user, RankingNameHolder::sSceneObjId));
}
