#pragma once

#include "Library/Area/AreaObjFactory.h"

class ProjectAreaFactory : public al::AreaObjFactory {
public:
    ProjectAreaFactory();
};

static_assert(sizeof(ProjectAreaFactory) == 0x30);
