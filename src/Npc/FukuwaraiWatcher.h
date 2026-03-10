#pragma once

#include <container/seadPtrArray.h>

#include "Library/LiveActor/LiveActor.h"
#include "Library/LiveActor/LiveActorGroup.h"

namespace al {
class FixMapParts;
}  // namespace al

class FukuwaraiFaceParts;
class FukuwaraiNpc;

class FukuwaraiWatcher : public al::LiveActor {
public:
    FukuwaraiWatcher(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void control() override;

    s32 getPartsNum() const;
    FukuwaraiFaceParts* getParts(s32 index) const;
    s32 calcTotalScore() const;

    void exeWait();
    void exeSetStartPosition();
    void exeMemorize();
    void exePlay();
    void exeWaitStartResultEnd();
    void exeResultWait();
    void exeResultAppearParts();
    void exeEnd();

private:
    al::DeriveActorGroup<FukuwaraiFaceParts>* mPartsGroup = nullptr;
    sead::FixedPtrArray<FukuwaraiFaceParts, 30> mSortedParts;
    FukuwaraiNpc* mNpc = nullptr;
    al::FixMapParts* mFace = nullptr;
    al::FixMapParts* mFaceLine = nullptr;
    al::FixMapParts* mFaceSilhouette = nullptr;
    bool mIsMarioFace = false;
};

static_assert(sizeof(FukuwaraiWatcher) == 0x238);
