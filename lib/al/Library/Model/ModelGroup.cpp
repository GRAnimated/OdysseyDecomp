#include "Library/Model/ModelGroup.h"

namespace al {

ModelGroup::ModelGroup(s32 maxModels) {
    mArray.allocBuffer(maxModels, nullptr);
}

ModelGroup::~ModelGroup() {
    mArray.freeBuffer();
}

void ModelGroup::registerModel(ModelKeeper* model) {
    mArray.pushBack(model);
}

}  // namespace al
