#pragma once

#include "Library/Message/ReplaceTagProcessorBase.h"

namespace al {
class IUseSceneObjHolder;
}

class ProjectReplaceTagProcessor : public al::ReplaceTagProcessorBase {
public:
    ProjectReplaceTagProcessor(const al::IUseSceneObjHolder* holder);

private:
    const al::IUseSceneObjHolder* mSceneObjHolder;
};
