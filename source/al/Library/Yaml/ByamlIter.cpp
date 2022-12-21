#include "al/Library/Yaml/ByamlIter.h"

namespace al {

    bool ByamlIter::tryGetUInt64ByIndex(u64 *pOut, int idx) const {
        ByamlData byml;

        if (!getByamlDataByIndex(&byml, idx)) {
            return false;
        }

        if (byml.getType() == 0xFF) {
            return false;
        }

        return tryConvertUInt64(pOut, &byml);
    }

    bool ByamlIter::tryGetDoubleByIndex(f64 *pOut, int idx) const {
        ByamlData byml;

        if (!getByamlDataByIndex(&byml, idx)) {
            return false;
        }

        if (byml.getType() == 0xFF) {
            return false;
        }

        // this call gets inlined
        return tryConvertDouble(pOut, &byml);
    }

    bool ByamlIter::tryConvertDouble(f64 *pOut, const ByamlData *pData) const {
        u32 val = pData->getValue();

        if (pData->getType() == 0xD2) {
            *pOut = *reinterpret_cast<f32*>(&val);
            return true;
        }

        if (pData->getType() == 0xD6) {
            u64 dbl_val = alByamlLocalUtil::getData64Bit(mData, val, mHeader->isInvertOrder());
            *pOut = *reinterpret_cast<f64*>(&dbl_val);
            return true;
        }

        return false;
    }

    bool ByamlIter::tryConvertIter(ByamlIter *pIter, const ByamlData *pData) const {
        if (pData->getType() == 0xC0 || pData->getType() == 0xC1) {
            pIter->set(mData, &mData[pData->getValue()]);
            return true;
        }

        if (pData->getType() == 0xFF) {
            pIter->set(mData, nullptr);
            return true;
        }

        return false;
    }

    bool ByamlIter::isEqualData(const ByamlIter &rIter) const {
        if (mData == nullptr) {
            return false;
        }

        bool ret = false;

        if (rIter.mData != nullptr) {
            if (mData == rIter.mData) {
                ret = mRootNode == rIter.mRootNode;
            }
            
        }

        return ret;
    }

    u32 ByamlContainerHeader::getType() const {
        return mData;
    }

    u32 ByamlContainerHeader::getCount(bool isRev) const {
        if (isRev) {
            return (mData >> 24) | (mData >> 8 & 0xFF00) | (mData << 8 & 0xFF0000);
        }

        return (mData >> 8);
    }
};