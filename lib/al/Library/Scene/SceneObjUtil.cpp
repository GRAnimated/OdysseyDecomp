#include "Library/Scene/SceneObjUtil.h"

#include "Library/Scene/ISceneObj.h"
#include "Library/Scene/IUseSceneObjHolder.h"
#include "Library/Scene/SceneObjHolder.h"

namespace al {
ISceneObj* createSceneObj(const al::IUseSceneObjHolder* user, s32 sceneObjId) {
    return user->getSceneObjHolder()->create(sceneObjId);
}

void setSceneObj(const al::IUseSceneObjHolder* user, al::ISceneObj* obj, s32 sceneObjId) {
    user->getSceneObjHolder()->setSceneObj(obj, sceneObjId);
}

ISceneObj* getSceneObj(const al::IUseSceneObjHolder* user, s32 sceneObjId) {
    return user->getSceneObjHolder()->getObj(sceneObjId);
}

ISceneObj* tryGetSceneObj(const al::IUseSceneObjHolder* user, s32 sceneObjId) {
    return user->getSceneObjHolder()->tryGetObj(sceneObjId);
}

bool isExistSceneObj(const al::IUseSceneObjHolder* user, s32 sceneObjId) {
    return user->getSceneObjHolder()->isExist(sceneObjId);
}

void deleteSceneObj(const al::IUseSceneObjHolder* user, s32 sceneObjId) {
    delete user->getSceneObjHolder()->getObj(sceneObjId);
}

bool tryDeleteSceneObj(const al::IUseSceneObjHolder* user, s32 sceneObjId) {
    ISceneObj* sceneObj = tryGetSceneObj(user, sceneObjId);
    if (sceneObj) {
        delete sceneObj;
        return true;
    }
    return false;
}
}  // namespace al
