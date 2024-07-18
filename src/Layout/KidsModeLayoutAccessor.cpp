#include "Layout/KidsModeLayoutAccessor.h"
#include "Library/Scene/SceneObjUtil.h"
#include "Scene/SceneObjFactory.h"
#include "System/GameDataHolderAccessor.h"

KidsModeLayoutAccessor::KidsModeLayoutAccessor() {}

const char* KidsModeLayoutAccessor::getSceneObjName() const {
    return "キッズモードレイアウトアクセサ";
}

namespace rs {
void setKidsModeLayoutDisable(const al::IUseSceneObjHolder* holder) {
    al::getSceneObj<KidsModeLayoutAccessor>(holder, SceneObjID_KidsModeLayoutAccessor)
        ->mIsKidsModeLayoutDisable = true;
}

void setKidsModeLayoutEnable(const al::IUseSceneObjHolder* holder) {
    al::getSceneObj<KidsModeLayoutAccessor>(holder, SceneObjID_KidsModeLayoutAccessor)
        ->mIsKidsModeLayoutDisable = false;
}

bool isKidsModeLayoutDisable(const al::IUseSceneObjHolder* holder) {
    return !rs::isKidsMode(holder) ||
           al::getSceneObj<KidsModeLayoutAccessor>(holder, SceneObjID_KidsModeLayoutAccessor)
               ->mIsKidsModeLayoutDisable;
}

}  // namespace rs
