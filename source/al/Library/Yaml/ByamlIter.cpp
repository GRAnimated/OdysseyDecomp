#include "al/Library/Yaml/ByamlIter.h"

namespace al {
    u32 ByamlContainerHeader::getType() const { return mData; }

    u32 ByamlContainerHeader::getCount(bool isRev) const {
        if (isRev) {
            return (mData >> 24) | (mData >> 8 & 0xFF00) | (mData << 8 & 0xFF0000);
        }

        return (mData >> 8);
    }
};    // namespace al