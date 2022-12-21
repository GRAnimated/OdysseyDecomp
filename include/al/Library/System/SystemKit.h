#pragma once

#include <basis/seadTypes.h>
#include <heap/seadHeap.h>

using uint = u32;

namespace al {
    class MemorySystem;
    class FileLoader;
    class ResourceSystem;
    class SaveDataDirector;

    class SystemKit {
    public:
        SystemKit();
        MemorySystem *createMemorySystem(sead::Heap *heap);
        FileLoader *createFileLoader(int priority);
        ResourceSystem *createResourceSystem(char const *archiveLocation, int priority, int heapSize, bool isSzs);
        SaveDataDirector *createSaveDataSystem(uint, int priority);

        MemorySystem *mMemorySystem;
        FileLoader *mFileLoader;
        ResourceSystem *mResourceSystem;
        SaveDataDirector *mSaveDataDirector;
    };

}    // namespace al