#pragma once

#include <basis/seadTypes.h>

namespace al {
class ByamlIter;
class MaterialCategoryKeeper;
class ModelCtrl;
class Resource;

class ModelMaterialCategory {
public:
    ModelMaterialCategory(ModelCtrl*, MaterialCategoryKeeper*);
    void init(const char*);
    void init(const ByamlIter&);
    s32 getCategoryIdFromMaterialName(const char*) const;
    s32 getCategoryIdFromMaterialIndex(s32) const;
    static void tryCreate(ModelCtrl*, const Resource*, const char*, MaterialCategoryKeeper*);
    static void tryCreate(ModelCtrl*, const char*, MaterialCategoryKeeper*);

private:
    void* filler[4];
};
}  // namespace al
