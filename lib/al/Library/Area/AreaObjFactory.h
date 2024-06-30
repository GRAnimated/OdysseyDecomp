#pragma once

#include "Library/Factory/Factory.h"

namespace al {
class AreaObj;

class AreaObjFactory : public al::Factory<al::AreaObj* (*)()> {};
}  // namespace al