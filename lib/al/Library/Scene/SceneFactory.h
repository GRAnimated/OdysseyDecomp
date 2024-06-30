#pragma once

#include "Library/Factory/Factory.h"

namespace al {
class Scene;
}  // namespace al

namespace alSceneFunction {
using SceneCreatorFunction = al::Scene* (*)();

template <typename T>
al::Scene* createSceneFunc() {
    return new T();
}

class SceneFactory : public al::Factory<SceneCreatorFunction> {
public:
    SceneFactory(const char* name);
};
}  // namespace alSceneFunction
