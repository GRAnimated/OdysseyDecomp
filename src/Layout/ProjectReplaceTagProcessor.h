#pragma once

#include "Library/Layout/ReplaceTagProcessor.h"

namespace al {
class IUseSceneObjHolder;
}

class ProjectReplaceTagProcessor : public al::ReplaceTagProcessorBase {
public:
    ProjectReplaceTagProcessor(const al::IUseSceneObjHolder* holder);

    u32 replaceProjectTag(char16* out, const al::MessageTag& tag,
                          const al::IUseMessageSystem* msgSys) const override;

    u32 replace(char16_t* out, const al::IUseMessageSystem* msgSys, const char16_t* in) const;

private:
    const al::IUseSceneObjHolder* mSceneObjHolder;
};

static_assert(sizeof(ProjectReplaceTagProcessor) == 0x10);
