#pragma once

#include "Library/Factory/Factory.h"
namespace al {
class CameraPoser;

class CameraPoserFactory : public al::Factory<al::CameraPoser*(*)> {};
}  // namespace al