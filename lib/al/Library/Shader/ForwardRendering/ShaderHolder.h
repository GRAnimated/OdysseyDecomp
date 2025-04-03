#pragma once

#include <heap/seadDisposer.h>

#include "Library/HostIO/HioNode.h"

namespace al {

class ShaderHolder : public HioNode {
    SEAD_SINGLETON_DISPOSER(ShaderHolder)
public:
    ShaderHolder();

    class ShaderArchiveSuffix {
    public:
        ShaderArchiveSuffix(const char*, const char*);

    private:
        void* filler[15];
    };

    void init();
    void initAndLoadAllFromDir(const char*, sead::Heap*, sead::Heap*);
    void loadAll(const char*, sead::Heap*, const char*);
    void load(const char*, const char*, sead::Heap*, u32);
    void tryGetShaderProgram(const char*) const;
    void getShaderProgram(const char*) const;
    void getShadingModel(const char*, const char*) const;

private:
    void* filler[103];
};

}  // namespace al

static_assert(sizeof(al::ShaderHolder::ShaderArchiveSuffix) == 0x78);
static_assert(sizeof(al::ShaderHolder) == 0x358);
