#include "Library/Collision/KCollisionServer.h"

#include <prim/seadMemUtil.h>

#include "Library/Math/MathUtil.h"
#include "Library/Yaml/ByamlIter.h"

namespace al {

KCollisionServer::KCollisionServer() {
    sead::MemUtil::fillZero(this, 0x48);
    mFarthestVertexDistance = 1.0f;
}
void KCollisionServer::initKCollisionServer(void* data, const void* attributeData) {
    setData(data);
    if (attributeData)
        mAttributeIter = new ByamlIter(static_cast<const u8*>(attributeData));
}
void KCollisionServer::setData(void* data) {
    mData = static_cast<const KCollisionServerData*>(data);
    mModelListData = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(mData) +
                                             mData->modelListDataOffset);
    mOctreeData = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(mData) +
                                          mData->octreeDataOffset);

    mAreaWidthShift[0] = mData->widthShift[0] - 1;
    mAreaWidthShift[1] = mData->widthShift[1] - 1;
    mAreaWidthShift[2] = mData->widthShift[2] - 1;
    mAreaWidthMask[0] = 0xffffffffu << mData->widthShift[0];
    mAreaWidthMask[1] = 0xffffffffu << mData->widthShift[1];
    mAreaWidthMask[2] = 0xffffffffu << mData->widthShift[2];

    mModelsData.allocBuffer(mData->modelCount, nullptr);
    for (s32 i = 0; i < mData->modelCount; i++) {
        const u32* modelOffsets = static_cast<const u32*>(mModelListData);
        const char* dataBytes = reinterpret_cast<const char*>(mData);
        const KCPrismHeader* header =
            reinterpret_cast<const KCPrismHeader*>(dataBytes + modelOffsets[i]);
        mModelsData.pushBack(const_cast<KCPrismHeader*>(header));
    }
}
const KCPrismHeader& KCollisionServer::getInnerKcl(s32 index) const {
    const u32* offsets = static_cast<const u32*>(mModelListData);
    return *reinterpret_cast<const KCPrismHeader*>(reinterpret_cast<const char*>(mData) +
                                                   offsets[index]);
}
u32 KCollisionServer::getNumInnerKcl() const {
    return mData->modelCount;
}
const KCPrismHeader* KCollisionServer::getV1Header(s32 index) const {
    if (u32(mModelsData.size()) <= u32(index))
        return nullptr;
    return mModelsData(index);
}
u64 KCollisionServer::getTriangleNum(const KCPrismHeader* header) const {
    return (static_cast<u64>(header->octreeOffset) - header->trianglesOffset) /
           sizeof(KCPrismData);
}
const KCPrismData& KCollisionServer::getPrismData(u32 index, const KCPrismHeader* header) const {
    uintptr_t address = header->trianglesOffset;
    address += reinterpret_cast<uintptr_t>(header);
    return reinterpret_cast<const KCPrismData*>(address)[static_cast<s32>(index)];
}
bool KCollisionServer::isNearParallelNormal(const KCPrismData* data,
                                            const KCPrismHeader* header) const {
    sead::Vector3f edge1 = getEdgeNormal1(data, header);
    sead::Vector3f edge2 = getEdgeNormal2(data, header);
    sead::Vector3f edge3 = getEdgeNormal3(data, header);
    return isParallelDirection(edge1, edge2) || isParallelDirection(edge1, edge3) ||
           isParallelDirection(edge2, edge3);
}
void KCollisionServer::getMinMax(sead::Vector3f* min, sead::Vector3f* max) const {
    f32 minX = mData->min.x;
    f32 minY = mData->min.y;
    f32 minZ = mData->min.z;
    min->set(minX, minY, minZ);
    f32 maxX = mData->max.x;
    f32 maxY = mData->max.y;
    f32 maxZ = mData->max.z;
    max->set(maxX, maxY, maxZ);
}
void KCollisionServer::getAreaSpaceSize(sead::Vector3f* size,
                                            const KCPrismHeader* header) const {
    s32 x = ~header->widthMask.x;
    s32 y = ~header->widthMask.y;
    s32 z = ~header->widthMask.z;
    sead::Vector3f areaSize{f32(x), f32(y), f32(z)};
    *size = areaSize;
}
void KCollisionServer::getAreaSpaceSize(s32* sizeX, s32* sizeY, s32* sizeZ,
                                        const KCPrismHeader* header) const {
    *sizeX = ~header->widthMask.x;
    *sizeY = ~header->widthMask.y;
    *sizeZ = ~header->widthMask.z;
}
void KCollisionServer::getAreaSpaceSize(sead::Vector3u* size,
                                        const KCPrismHeader* header) const {
    size->x = ~header->widthMask.x;
    size->y = ~header->widthMask.y;
    size->z = ~header->widthMask.z;
}
// NON_MATCHING: behavior is corpus-complete but the current typed octree traversal still differs in the initial sentinel-check lowering; target keeps two sequential widthShift.z/y CMN checks. Next source-level hypothesis: recover a named octree-node representation that preserves the target sentinel checks and byte-offset descent without raw byte storage.
const u8* KCollisionServer::searchBlock(s32* widthShift, const sead::Vector3u& block,
                                        const KCPrismHeader* header) const {
    u32 shift = header->widthShift.x;
    *widthShift = shift;
    const u32* blockData = reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(header) +
                                                        header->octreeOffset);
    u32 offset = calcAreaBlockOffset(block, header);
    if (header->widthShift.z == 0xffffffffu)
        offset = header->widthShift.y == 0xffffffffu ? 0 : offset;

    u32 entry = getBlockData(blockData, offset);
    while ((entry & 0x80000000u) == 0) {
        --shift;
        *widthShift = shift;
        blockData = reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(blockData) + entry);
        entry = getBlockData(blockData, calcChildBlockOffset(block, shift));
    }
    return reinterpret_cast<const u8*>(reinterpret_cast<uintptr_t>(blockData) +
                                       (entry & 0x7fffffffu));
}

void KCollisionServer::objectSpaceToAreaOffsetSpaceV3f(sead::Vector3f* areaOff,
                                                         const sead::Vector3f& objSpace,
                                                         const KCPrismHeader* header) const {
    *areaOff = objSpace - header->octreeOrigin;
}
bool KCollisionServer::isInsideMinMaxInAreaOffsetSpace(
    const sead::Vector3u& block, const KCPrismHeader* header) const {
    if ((block.x & header->widthMask.x) != 0)
        return false;
    if ((block.y & header->widthMask.y) != 0)
        return false;
    return (block.z & header->widthMask.z) == 0;
}
// NON_MATCHING: behavior is corpus-complete at 220/244; local vector copies reproduce the target load/FCCMP/FNEG sequence, but Clang tail-merges the first five true returns into one MOV/RET while the target keeps five separate return blocks. Next source-level hypothesis: recover the original comparison source shape that inhibits return-block merging without attributes or artificial liveness.
bool KCollisionServer::isParallelNormal(const KCPrismData* data,
                                        const KCPrismHeader* header) const {
    sead::Vector3f edge1 = getEdgeNormal1(data, header);
    sead::Vector3f edge2 = getEdgeNormal2(data, header);
    if (edge1 == edge2)
        return true;
    sead::Vector3f edge3 = getEdgeNormal3(data, header);
    if (edge1 == edge3 || edge2 == edge3)
        return true;
    if (edge1.x == -edge2.x && edge1.y == -edge2.y && edge1.z == -edge2.z)
        return true;
    if (edge1.x == -edge3.x && edge1.y == -edge3.y && edge1.z == -edge3.z)
        return true;
    return edge2.x == -edge3.x && edge2.y == -edge3.y && edge2.z == -edge3.z;
}
const sead::Vector3f& KCollisionServer::getFaceNormal(const KCPrismData* data,
                                                      const KCPrismHeader* header) const {
    uintptr_t index = data->faceNormalIndex;
    uintptr_t offset = header->normalsOffset;
    return *reinterpret_cast<const sead::Vector3f*>(
        reinterpret_cast<uintptr_t>(header) + offset + index * sizeof(sead::Vector3f));
}
const sead::Vector3f& KCollisionServer::getEdgeNormal1(const KCPrismData* data,
                                                       const KCPrismHeader* header) const {
    uintptr_t index = data->edgeNormalIndex[0];
    uintptr_t offset = header->normalsOffset;
    return *reinterpret_cast<const sead::Vector3f*>(
        reinterpret_cast<uintptr_t>(header) + offset + index * sizeof(sead::Vector3f));
}
const sead::Vector3f& KCollisionServer::getEdgeNormal2(const KCPrismData* data,
                                                       const KCPrismHeader* header) const {
    uintptr_t index = data->edgeNormalIndex[1];
    uintptr_t offset = header->normalsOffset;
    return *reinterpret_cast<const sead::Vector3f*>(
        reinterpret_cast<uintptr_t>(header) + offset + index * sizeof(sead::Vector3f));
}
const sead::Vector3f& KCollisionServer::getEdgeNormal3(const KCPrismData* data,
                                                       const KCPrismHeader* header) const {
    uintptr_t index = data->edgeNormalIndex[2];
    uintptr_t offset = header->normalsOffset;
    return *reinterpret_cast<const sead::Vector3f*>(
        reinterpret_cast<uintptr_t>(header) + offset + index * sizeof(sead::Vector3f));
}
s32 KCollisionServer::toIndex(const KCPrismData* data, const KCPrismHeader* header) const {
    const KCPrismData* offsetData = reinterpret_cast<const KCPrismData*>(
        reinterpret_cast<uintptr_t>(data) - header->trianglesOffset);
    return offsetData - reinterpret_cast<const KCPrismData*>(header);
}
const sead::Vector3f& KCollisionServer::getNormal(u32 index, const KCPrismHeader* header) const {
    uintptr_t address = header->normalsOffset;
    address += reinterpret_cast<uintptr_t>(header);
    s32 signedIndex = index;
    return reinterpret_cast<const sead::Vector3f*>(address)[signedIndex];
}
void KCollisionServer::calXvec(const sead::Vector3f* a, const sead::Vector3f* b,
                                  sead::Vector3f* result) {
    result->x = a->z * b->y - a->y * b->z;
    result->y = a->x * b->z - a->z * b->x;
    result->z = a->y * b->x - a->x * b->y;
}
const sead::Vector3f& KCollisionServer::getVertexData(u32 index,
                                                      const KCPrismHeader* header) const {
    uintptr_t address = header->positionsOffset;
    address += reinterpret_cast<uintptr_t>(header);
    s32 signedIndex = index;
    return reinterpret_cast<const sead::Vector3f*>(address)[signedIndex];
}
u32 KCollisionServer::getVertexNum(const KCPrismHeader* header) const {
    uintptr_t normalsAddress = header->normalsOffset;
    normalsAddress += reinterpret_cast<uintptr_t>(header);
    uintptr_t positionsAddress = header->positionsOffset;
    positionsAddress += reinterpret_cast<uintptr_t>(header);
    return reinterpret_cast<const sead::Vector3f*>(normalsAddress) -
           reinterpret_cast<const sead::Vector3f*>(positionsAddress);
}
s32 KCollisionServer::getNormalNum(const KCPrismHeader* header) const {
    return getTriangleNum(header) * 4;
}
s32 KCollisionServer::getAttributeElementNum() const {
    return 0;
}
bool KCollisionServer::getAttributes(ByamlIter* destIter, u32 triIndex,
                                     const KCPrismHeader* header) const {
    return mAttributeIter->tryGetIterByIndex(destIter, getPrismData(triIndex, header).collisionType);
}
bool KCollisionServer::getAttributes(ByamlIter* destIter, const KCPrismData* data) const {
    return mAttributeIter->tryGetIterByIndex(destIter, data->collisionType);
}
void KCollisionServer::objectSpaceToAreaOffsetSpace(sead::Vector3u* areaOffSpace,
                                                    const sead::Vector3f& objSpace,
                                                    const KCPrismHeader* header) const {
    areaOffSpace->x = static_cast<s32>(objSpace.x - header->octreeOrigin.x);
    areaOffSpace->y = static_cast<s32>(objSpace.y - header->octreeOrigin.y);
    areaOffSpace->z = static_cast<s32>(objSpace.z - header->octreeOrigin.z);
}
void KCollisionServer::areaOffsetSpaceToObjectSpace(sead::Vector3f* objSpace,
                                                    const sead::Vector3u& areaOffSpace,
                                                    const KCPrismHeader* header) const {
    f32 x = static_cast<f32>(areaOffSpace.x) + header->octreeOrigin.x;
    f32 y = static_cast<f32>(areaOffSpace.y) + header->octreeOrigin.y;
    f32 z = static_cast<f32>(areaOffSpace.z) + header->octreeOrigin.z;
    sead::Vector3f result(x, y, z);
    *objSpace = result;
}
// NON_MATCHING: behavior and size are target-complete at 52/52, but current source groups the initial block loads as X/Y while target starts with Y/Z and schedules the width-shift loads differently. Next source-level hypothesis: recover the original packed/block-coordinate source shape without pointer-cast or liveness hacks.
s32 KCollisionServer::calcAreaBlockOffset(const sead::Vector3u& block,
                                          const KCPrismHeader* header) const {
    u32 y = block.y;
    u32 z = block.z;
    u32 shiftZ = header->widthShift.z;
    u32 shiftX = header->widthShift.x;
    u32 shiftY = header->widthShift.y;
    u32 zPart = (z >> shiftX) << shiftZ;
    u32 x = block.x;
    u32 yPart = (y >> shiftX) << shiftY;
    return 4 * ((yPart | zPart) | (x >> shiftX));
}
s32 KCollisionServer::calcChildBlockOffset(const sead::Vector3u& block, s32 shift) {
    return 4 * ((4 * ((block.z >> shift) & 1)) | (2 * ((block.y >> shift) & 1)) |
                ((block.x >> shift) & 1));
}
u32 KCollisionServer::getBlockData(const u32* data, u32 offset) {
    return *reinterpret_cast<const u32*>(reinterpret_cast<uintptr_t>(data) + offset);
}

void SphereInterpolator::startInterp(const sead::Vector3f& posStart, const sead::Vector3f& posEnd,
                                     f32 sizeStart, f32 sizeEnd, f32 steps) {
    mCurrentStep = 0.0f;
    mPrevStep = 0.0f;
    mPos = posStart;
    mMove = posEnd - posStart;
    mSizeStart = sizeStart;
    mSizeEnd = sizeEnd;

    f32 dist = mMove.length() + sizeEnd - sizeStart;
    mStepSize = (dist <= 0.0f) ? 1.0f : steps / dist;
}

void SphereInterpolator::nextStep() {
    // re-interpreting between f32/s32 required to match
    s32 curStep = *(s32*)&mCurrentStep;
    f32 stepAsFloat = *(f32*)&curStep;
    f32 newStep = sead::Mathf::clampMax(stepAsFloat + mStepSize, 1.0f);
    *(s32*)&mPrevStep = curStep;
    mCurrentStep = newStep;
}

void SphereInterpolator::calcInterpPos(sead::Vector3f* pos) const {
    f32 step = mCurrentStep;
    pos->x = mMove.x * step + mPos.x;
    pos->y = mMove.y * step + mPos.y;
    pos->z = mMove.z * step + mPos.z;
}

void SphereInterpolator::calcInterp(sead::Vector3f* pos, f32* size,
                                    sead::Vector3f* remainMoveVec) const {
    calcInterpPos(pos);
    *size = mSizeStart + (mSizeEnd - mSizeStart) * mCurrentStep;
    calcRemainMoveVector(remainMoveVec);
}

void SphereInterpolator::calcRemainMoveVector(sead::Vector3f* remainMoveVec) const {
    if (remainMoveVec) {
        f32 remainStep = 1.0f - mCurrentStep;
        remainMoveVec->x = mMove.x * remainStep;
        remainMoveVec->y = mMove.y * remainStep;
        remainMoveVec->z = mMove.z * remainStep;
    }
}

void SphereInterpolator::getMoveVector(sead::Vector3f* moveVec) {
    f32 step = mCurrentStep;
    moveVec->x = mMove.x * step;
    moveVec->y = mMove.y * step;
    moveVec->z = mMove.z * step;
}

void SphereInterpolator::calcStepMoveVector(sead::Vector3f* moveVec) const {
    f32 step = mCurrentStep - mPrevStep;
    moveVec->x = mMove.x * step;
    moveVec->y = mMove.y * step;
    moveVec->z = mMove.z * step;
}

void SpherePoseInterpolator::startInterp(const sead::Vector3f& posStart,
                                         const sead::Vector3f& posEnd, f32 sizeStart, f32 sizeEnd,
                                         const sead::Quatf& quatStart, const sead::Quatf& quatEnd,
                                         f32 steps) {
    mCurrentStep = 0.0f;
    mPrevStep = 0.0f;
    mPos = posStart;
    mMove = posEnd - posStart;

    mQuatStart.x = quatStart.x;
    mQuatStart.y = quatStart.y;
    mQuatStart.z = quatStart.z;
    mQuatStart.w = quatStart.w;

    mQuatEnd.x = quatEnd.x;
    mQuatEnd.y = quatEnd.y;
    mQuatEnd.z = quatEnd.z;
    mQuatEnd.w = quatEnd.w;

    mSizeStart = sizeStart;
    mSizeEnd = sizeEnd;

    f32 dist = mMove.length() + sizeEnd - sizeStart;
    mStepSize = (dist <= 0.0f) ? 1.0f : steps / dist;
}

void SpherePoseInterpolator::nextStep() {
    // re-interpreting between f32/s32 required to match
    s32 curStep = *(s32*)&mCurrentStep;
    f32 stepAsFloat = *(f32*)&curStep;
    f32 newStep = sead::Mathf::clampMax(stepAsFloat + mStepSize, 1.0f);
    *(s32*)&mPrevStep = curStep;
    mCurrentStep = newStep;
}

void SpherePoseInterpolator::calcInterpPos(sead::Vector3f* pos) const {
    f32 step = mCurrentStep;
    pos->x = mMove.x * step + mPos.x;
    pos->y = mMove.y * step + mPos.y;
    pos->z = mMove.z * step + mPos.z;
}

void SpherePoseInterpolator::calcInterp(sead::Vector3f* pos, f32* size, sead::Quatf* quat,
                                        sead::Vector3f* remainMoveVec) const {
    calcInterpPos(pos);
    *size = mSizeStart + (mSizeEnd - mSizeStart) * mCurrentStep;
    slerpQuat(quat, mQuatStart, mQuatEnd, mCurrentStep);
    quat->normalize();
    calcRemainMoveVector(remainMoveVec);
}

void SpherePoseInterpolator::calcRemainMoveVector(sead::Vector3f* remainMoveVec) const {
    if (remainMoveVec) {
        f32 remainStep = 1.0f - mCurrentStep;
        remainMoveVec->x = mMove.x * remainStep;
        remainMoveVec->y = mMove.y * remainStep;
        remainMoveVec->z = mMove.z * remainStep;
    }
}

f32 SpherePoseInterpolator::calcRadiusBaseScale(f32 radius) const {
    return calcRate01(radius, 0.0f, mSizeEnd);
}

void SpherePoseInterpolator::getMoveVector(sead::Vector3f* moveVec) {
    f32 step = mCurrentStep;
    moveVec->x = mMove.x * step;
    moveVec->y = mMove.y * step;
    moveVec->z = mMove.z * step;
}

}  // namespace al
