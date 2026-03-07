#pragma once

#include "Library/Camera/CameraPoserFactory.h"

class ProjectCameraPoserFactory : public al::CameraPoserFactory {
public:
    ProjectCameraPoserFactory();
};

static_assert(sizeof(ProjectCameraPoserFactory) == 0x20);
