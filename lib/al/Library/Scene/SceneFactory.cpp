#include "Library/Scene/SceneFactory.h"

namespace alSceneFunction {
SceneFactory::SceneFactory(const char* factoryName) : Factory<al::Scene* (*)()>(factoryName) {}
}  // namespace alSceneFunction
