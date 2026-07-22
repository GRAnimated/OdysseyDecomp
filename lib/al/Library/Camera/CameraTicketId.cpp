#include "Library/Camera/CameraTicketId.h"

#include "Library/Base/StringUtil.h"
#include "Library/Placement/PlacementId.h"
#include "Library/Yaml/ByamlUtil.h"

namespace al {

CameraTicketId::CameraTicketId(const PlacementId* placementId, const char* suffix)
    : mPlacementId(placementId), mSuffix(suffix) {}

bool CameraTicketId::isEqual(const CameraTicketId& other) const {
    if (!mPlacementId && other.mPlacementId)
        return false;
    if (mPlacementId && !other.mPlacementId)
        return false;

    if (!mPlacementId && !other.mPlacementId)
        return isEqualString(mSuffix, other.mSuffix);
    if (!mPlacementId->isEqual(*other.mPlacementId))
        return false;

    bool noneSuffix = !mSuffix && !other.mSuffix;
    if (mSuffix && other.mSuffix)
        return isEqualString(mSuffix, other.mSuffix);

    return noneSuffix;
}

bool CameraTicketId::isEqual(const CameraTicketId& ticket1, const CameraTicketId& ticket2) {
    return ticket1.isEqual(ticket2);
}

bool CameraTicketId::isEqual(const ByamlIter& iter) const {
    const char* id = getObjId();
    const char* otherId = tryGetByamlKeyStringOrNULL(iter, "ObjId");
    if (id && otherId) {
        if (__builtin_expect(!isEqualString(id, otherId), 0))
            return false;
    } else if (__builtin_expect(id != nullptr, 0) || __builtin_expect(otherId != nullptr, 0)) {
        return false;
    }

    const char* suffix = mSuffix;
    const char* otherSuffix = tryGetByamlKeyStringOrNULL(iter, "Suffix");
    if (__builtin_expect(suffix && otherSuffix, 1))
        return isEqualString(suffix, otherSuffix);
    return !suffix && !otherSuffix;
}

const char* CameraTicketId::tryGetObjId() const {
    if (!mPlacementId)
        return nullptr;
    return mPlacementId->getId();
}

const char* CameraTicketId::getObjId() const {
    return tryGetObjId();
}

}  // namespace al
