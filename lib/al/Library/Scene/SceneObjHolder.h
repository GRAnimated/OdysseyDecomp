#pragma once

#include <basis/seadTypes.h>

namespace al {
class ActorInitInfo;
class ISceneObj;
class IUseSceneObjHolder;
class PlayerHolder;
class Scene;

class SceneObjHolder {
public:
    SceneObjHolder(ISceneObj* (*)(s32), s32);
    ISceneObj* create(s32);
    ISceneObj* tryGetObj(s32) const;
    ISceneObj* getObj(s32) const;
    bool isExist(s32) const;
    void setSceneObj(ISceneObj*, s32);
    void initAfterPlacementSceneObj(const ActorInitInfo&);

private:
    ISceneObj* (*mCreator)(s32);
    ISceneObj** mSceneObjArray;
    s32 mArraySize;
};

ISceneObj* createSceneObj(const al::IUseSceneObjHolder*, s32);
void setSceneObj(const al::IUseSceneObjHolder*, al::ISceneObj*, s32);
ISceneObj* getSceneObj(const al::IUseSceneObjHolder*, s32);
ISceneObj* tryGetSceneObj(const al::IUseSceneObjHolder*, s32);
bool isExistSceneObj(const al::IUseSceneObjHolder*, s32);
void deleteSceneObj(const al::IUseSceneObjHolder*, s32);
bool tryDeleteSceneObj(const al::IUseSceneObjHolder*, s32);

}  // namespace al
