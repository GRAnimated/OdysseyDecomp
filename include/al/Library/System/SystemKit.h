#pragma once

#include <basis/seadTypes.h>
#include <heap/seadHeap.h>

<<<<<<< HEAD
=======

>>>>>>> 60112915b1b559e06290092c73b9c070ba03786b
using uint = u32;

namespace al {
    class MemorySystem;
    class FileLoader;
    class ResourceSystem;
    class SaveDataDirector;

    class SystemKit {
    public:
        SystemKit();
<<<<<<< HEAD
        MemorySystem *createMemorySystem(sead::Heap *heap);
        FileLoader *createFileLoader(int priority);
        ResourceSystem *createResourceSystem(char const *archiveLocation, int priority, int heapSize, bool isSzs);
        SaveDataDirector *createSaveDataSystem(uint, int priority);

        MemorySystem *mMemorySystem;
        FileLoader *mFileLoader;
        ResourceSystem *mResourceSystem;
        SaveDataDirector *mSaveDataDirector;
=======
        MemorySystem* createMemorySystem(sead::Heap* heap);
        FileLoader* createFileLoader(int priority);
        ResourceSystem* createResourceSystem(char const* archiveLocation, int priority, int heapSize, bool isSzs);
        SaveDataDirector* createSaveDataSystem(uint, int priority);

        MemorySystem* mMemorySystem;
        FileLoader* mFileLoader;
        ResourceSystem* mResourceSystem;
        SaveDataDirector* mSaveDataDirector;
>>>>>>> 60112915b1b559e06290092c73b9c070ba03786b
    };

}    // namespace al