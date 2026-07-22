#pragma once

#include <basis/seadTypes.h>

#include "Library/LiveActor/LiveActorInfo.h"

namespace al {
struct ActorInitInfo;
class LiveActor;

struct SubActorSync {
    enum Enum {
        cNone = 0,                                       // 0
        cAppear = 1 << 0,                                // 1
        cClipping = 1 << 1,                              // 2
        cHide = 1 << 2,                                  // 4
        cAlphaMask = 1 << 3,                             // 8
        cAll = cAppear | cClipping | cHide | cAlphaMask  // 15
    };

    AL_BITS(SubActorSync)
};

struct SubActorInfo {
public:
    SubActorInfo();

    SubActorInfo(LiveActor* actor, SubActorSync syncType) : subActor(actor), syncType(syncType) {}

    LiveActor* subActor = nullptr;
    void* field_8 = nullptr;
    SubActorSync syncType = SubActorSync::cNone;
};

class SubActorKeeper {
public:
    SubActorKeeper(LiveActor* rootActor);
    void registerSubActor(LiveActor* subActor, u32 syncType);
    void init(const ActorInitInfo& initInfo, const char* suffix, s32 maxSubActors);

    static SubActorKeeper* create(LiveActor* rootActor);
    static SubActorKeeper* tryCreate(LiveActor* rootActor, const char* suffix, s32 maxSubActors);

    s32 getCurActorCount() const { return mCurActorCount; }

    SubActorInfo* getActorInfo(s32 index) const { return mBuffer[index]; }

private:
    LiveActor* mRootActor;
    s32 mMaxActorCount = 0;
    s32 mCurActorCount = 0;
    SubActorInfo** mBuffer = nullptr;
};

}  // namespace al
