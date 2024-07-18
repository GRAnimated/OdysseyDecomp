#pragma once

#include "Library/Factory/Factory.h"

namespace al {
class CameraPoser;

class CameraPoserFactory : public Factory<CameraPoser*(*)> {};
}  // namespace al