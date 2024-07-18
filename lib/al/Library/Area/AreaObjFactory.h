#pragma once

#include "Library/Factory/Factory.h"

namespace al {
class AreaObj;

class AreaObjFactory : public Factory<AreaObj* (*)()> {};
}  // namespace al