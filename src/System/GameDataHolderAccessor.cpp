#include "System/GameDataHolderAccessor.h"
#include "Library/Scene/SceneObjHolder.h"
#include "Library/Scene/SceneObjUtil.h"

GameDataHolderAccessor::GameDataHolderAccessor(const al::IUseSceneObjHolder* holder) {
    mHolder = (GameDataHolder*)al::getSceneObj(holder, 18);
}

GameDataHolderAccessor::GameDataHolderAccessor(const al::SceneObjHolder* holder) {
    mHolder = (GameDataHolder*)holder->getObj(18);
}
