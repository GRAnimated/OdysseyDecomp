#include "Project/Item/ActorScoreKeeper.h"

#include "Library/Base/StringUtil.h"
#include "Library/Yaml/ByamlIter.h"

namespace al {
ActorScoreInfo::ActorScoreInfo() = default;

void ActorScoreInfo::init(const ByamlIter& iter) {
    iter.tryGetStringByKey(&mFactorName, "FactorName");
    iter.tryGetStringByKey(&mCategoryName, "CategoryName");
}

ActorScoreKeeper::ActorScoreKeeper() = default;

void ActorScoreKeeper::init(const ByamlIter& iter) {
    if (iter.isTypeArray()) {
        mSize = iter.getSize();
        mArray = new ActorScoreInfo[mSize];
        for (s32 i = 0; i < mSize; i++) {
            ByamlIter subIter;
            iter.tryGetIterByIndex(&subIter, i);
            mArray[i].init(subIter);
        }
    } else {
        mSize = 1;
        mArray = new ActorScoreInfo[mSize];
        mArray[0].init(iter);
    }
}

const char* ActorScoreKeeper::getCategoryName() const {
    return mArray[0].getCategoryName();
}

const char* ActorScoreKeeper::tryGetCategoryName(const char* factorName) const {
    for (s32 i = 0; i < mSize; i++)
        if (mArray->getFactorName() && isEqualString(mArray[i].getFactorName(), factorName))
            return mArray[i].getCategoryName();
    return nullptr;
}
}  // namespace al
