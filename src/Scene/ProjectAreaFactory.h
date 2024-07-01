#pragma once

#include "Library/Area/AreaObjFactory.h"

class ProjectAreaFactory : public al::AreaObjFactory {
public:
    ProjectAreaFactory();

    void* field_20;
    s32 field_28;
};