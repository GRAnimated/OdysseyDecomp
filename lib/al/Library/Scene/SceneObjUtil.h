#pragma once

#include <basis/seadTypes.h>

namespace al {
class IUseSceneObjHolder;
class ISceneObj;

ISceneObj* createSceneObj(const IUseSceneObjHolder*, s32);
void setSceneObj(const IUseSceneObjHolder*, ISceneObj*, s32);
ISceneObj* getSceneObj(const IUseSceneObjHolder*, s32);
ISceneObj* tryGetSceneObj(const IUseSceneObjHolder*, s32);
bool isExistSceneObj(const IUseSceneObjHolder*, s32);
void deleteSceneObj(const IUseSceneObjHolder*, s32);
bool tryDeleteSceneObj(const IUseSceneObjHolder*, s32);

template <typename T>
inline T* getSceneObj(const IUseSceneObjHolder* holder, s32 index) {
    return static_cast<T*>(getSceneObj(holder, index));
}

}  // namespace al