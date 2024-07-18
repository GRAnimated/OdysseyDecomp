#pragma once

#include "Library/HostIO/HioNode.h"
#include "Library/Scene/ISceneObj.h"

namespace al {
class IUseSceneObjHolder;
}

class KidsModeLayoutAccessor : public al::HioNode, public al::ISceneObj {
public:
    KidsModeLayoutAccessor();

    virtual const char* getSceneObjName() const override;

    bool mIsKidsModeLayoutDisable = false;
};

namespace rs {
void setKidsModeLayoutDisable(const al::IUseSceneObjHolder*);
void setKidsModeLayoutEnable(const al::IUseSceneObjHolder*);
bool isKidsModeLayoutDisable(const al::IUseSceneObjHolder*);
}  // namespace rs
