#pragma once

#include <basis/seadTypes.h>

namespace al {
class ByamlIter;

class ActorScoreInfo {
public:
    ActorScoreInfo();

    void init(const ByamlIter& iter);

    const char* getFactorName() const { return mFactorName; }
    const char* getCategoryName() const { return mCategoryName; }

private:
    const char* mFactorName = nullptr;
    const char* mCategoryName = nullptr;
};

static_assert(sizeof(ActorScoreInfo) == 0x10);

class ActorScoreKeeper {
public:
    ActorScoreKeeper();

    void init(const ByamlIter& iter);
    const char* getCategoryName() const;
    const char* tryGetCategoryName(const char* factorName) const;

private:
    ActorScoreInfo* mArray = nullptr;
    s32 mSize = 0;
};

static_assert(sizeof(ActorScoreKeeper) == 0x10);
}  // namespace al
