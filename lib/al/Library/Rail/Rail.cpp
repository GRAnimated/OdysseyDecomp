#include "Library/Rail/Rail.h"

#include "Library/Math/MathUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementInfo.h"
#include "Library/Rail/RailPart.h"

namespace al {

Rail::Rail() = default;

// NON_MATCHING: target keeps the rail-part count in X21 for new[] overflow sizing, then uses W21 and an offset-up constructor loop; a 64-bit source count fixes sizing but changes constructor traversal/lifetimes.
void Rail::init(const PlacementInfo& info) {
    mIsClosed = false;
    tryGetArg(&mIsClosed, info, "IsClosed");
    PlacementInfo railPointsInfo;
    tryGetPlacementInfoByKey(&railPointsInfo, info, "RailPoints");
    mRailPointsCount = getCountPlacementInfo(railPointsInfo);
    if (mRailPointsCount <= 0)
        return;

    mRailPoints = new PlacementInfo*[mRailPointsCount];
    for (s32 i = 0; i < mRailPointsCount; i++) {
        mRailPoints[i] = new PlacementInfo();
        tryGetPlacementInfoByIndex(mRailPoints[i], railPointsInfo, i);
    }

    if (mRailPointsCount == 1) {
        mRailPartCount = 1;
        mRailPart = new RailPart[1];
        PlacementInfo partInfo;
        tryGetPlacementInfoByIndex(&partInfo, railPointsInfo, 0);
        sead::Vector3f pos = sead::Vector3f::zero;
        tryGetRailPointPos(&pos, partInfo);
        mRailPart->init(pos, pos, pos, pos);
        return;
    }

    mRailPartCount = (mIsClosed ? 1 : 0) + mRailPointsCount - 1;
    mRailPart = new RailPart[mRailPartCount];

    f32 totalLength = 0;
    for (s32 i = 0; i < mRailPartCount; i++) {
        PlacementInfo startInfo, endInfo;
        tryGetPlacementInfoByIndex(&startInfo, railPointsInfo, i);
        tryGetPlacementInfoByIndex(&endInfo, railPointsInfo, (i + 1) % mRailPointsCount);

        sead::Vector3f start = sead::Vector3f::zero;
        sead::Vector3f startHandle = sead::Vector3f::zero;
        sead::Vector3f endHandle = sead::Vector3f::zero;
        sead::Vector3f end = sead::Vector3f::zero;
        tryGetRailPointPos(&start, startInfo);
        getRailPointHandleNext(&startHandle, startInfo);
        getRailPointHandlePrev(&endHandle, endInfo);
        tryGetRailPointPos(&end, endInfo);

        mRailPart[i].init(start, startHandle, endHandle, end);
        totalLength += mRailPart[i].getPartLength();
        mRailPart[i].setTotalDistance(totalLength);
    }
}

void Rail::calcPos(sead::Vector3f* pos, f32 distance) const {
    const RailPart* part = nullptr;
    f32 partDistance = 0;
    getIncludedSection(&part, &partDistance, distance);
    part->calcPos(pos, part->calcCurveParam(partDistance));
}

s32 Rail::getIncludedSection(const RailPart** part, f32* partDistance, f32 distance) const {
    f32 distanceOnRail = normalizeLength(distance);
    f32 startDistanceOnRail = 0.0;
    s32 maxRailPart = -1;
    s64 longI = -0x100000000;
    for (s32 i = 0; i < mRailPartCount; i++, longI += 0x100000000) {
        const s32 currentPart = i;
        if (distanceOnRail <= mRailPart[i].getTotalDistance()) {
            if (i > 0) {
                startDistanceOnRail = distanceOnRail - mRailPart[longI >> 32].getTotalDistance();
                maxRailPart = i;
                break;
            }
            maxRailPart = currentPart;
            startDistanceOnRail = distanceOnRail;
            break;
        }
    }

    if (part)
        *part = &mRailPart[maxRailPart];
    if (partDistance)
        *partDistance = sead::Mathf::clamp(startDistanceOnRail, 0.0, (*part)->getPartLength());

    return maxRailPart;
}

void Rail::calcDirection(sead::Vector3f* direction, f32 distance) const {
    const RailPart* part = nullptr;
    f32 partDistance = 0;
    getIncludedSection(&part, &partDistance, distance);
    part->calcDir(direction, part->calcCurveParam(partDistance));
}

void Rail::calcPosDir(sead::Vector3f* position, sead::Vector3f* direction, f32 distance) const {
    const RailPart* part = nullptr;
    f32 partDistance = 0;
    getIncludedSection(&part, &partDistance, distance);
    f32 curveParam = part->calcCurveParam(partDistance);
    part->calcPos(position, curveParam);
    part->calcDir(direction, curveParam);
}

f32 Rail::getTotalLength() const {
    return mRailPart[mRailPartCount - 1].getTotalDistance();
}

f32 Rail::getPartLength(s32 index) const {
    return mRailPart[index].getPartLength();
}

f32 Rail::getLengthToPoint(s32 index) const {
    if (index == 0)
        return 0;
    return mRailPart[index - 1].getTotalDistance();
}

void Rail::calcRailPointPos(sead::Vector3f* pos, s32 index) const {
    if (mIsClosed || index != mRailPointsCount - 1)
        return mRailPart[index].calcStartPos(pos);

    return mRailPart[index - 1].calcEndPos(pos);
}

void Rail::calcNearestRailPointPosFast(sead::Vector3f* rail_pos, u32* index,
                                       const sead::Vector3f& pos) const {
    u32 rail_points_count = mRailPointsCount;

    sead::Vector3f tmp;
    mRailPart[0].calcStartPos(&tmp);
    *rail_pos = tmp;
    *index = 0;

    f32 best_distance = (tmp - pos).squaredLength();
    u32 curr_index = 0;
    for (u32 i = 1; i < rail_points_count; i++) {
        mRailPart[curr_index].calcEndPos(&tmp);
        if ((tmp - pos).squaredLength() < best_distance) {
            best_distance = (tmp - pos).squaredLength();
            *rail_pos = tmp;
            *index = i;
        }
        curr_index += (i & 1);  // only increases every second iteration
    }
}

void Rail::calcNearestRailPointNo(s32* index, const sead::Vector3f& pos) const {
    sead::Vector3f tmp = sead::Vector3f::zero;
    calcRailPointPos(&tmp, 0);
    f32 best_distance = (pos - tmp).squaredLength();
    *index = 0;

    s32 curr_index = 1;
    for (s64 i = 1; i < mRailPointsCount; i++) {
        calcRailPointPos(&tmp, curr_index);
        if ((pos - tmp).squaredLength() < best_distance) {
            best_distance = (pos - tmp).squaredLength();
            *index = i;
        }
        curr_index++;
    }
}

void Rail::calcNearestRailPointPos(sead::Vector3f* rail_pos, const sead::Vector3f& pos) const {
    if (mRailPointsCount == 0)
        return;

    sead::Vector3f tmp = sead::Vector3f::zero;
    calcRailPointPos(&tmp, 0);
    f32 best_distance = (pos - tmp).squaredLength();

    s32 curr_index = 1;
    for (s64 i = 1; i < mRailPointsCount; i++) {
        if (!mIsClosed && curr_index == mRailPointsCount - 1)
            mRailPart[curr_index - 1].calcEndPos(&tmp);
        else
            mRailPart[curr_index].calcStartPos(&tmp);

        if ((pos - tmp).squaredLength() < best_distance) {
            best_distance = (pos - tmp).squaredLength();
            *rail_pos = tmp;
        }
        curr_index++;
    }
}

f32 Rail::normalizeLength(f32 distance) const {
    if (mIsClosed) {
        f32 distanceOnRail = modf(distance, getTotalLength());
        if (distanceOnRail < 0.0)
            distanceOnRail += getTotalLength();
        return distanceOnRail;
    }

    return sead::Mathf::clamp(distance, 0.0, getTotalLength());
}

// NON_MATCHING: body size/register sequence is exact; tools/check first differs at the shared anonymous FLT_MAX `.rodata.cst4` relocation (target 0x9c2a40).
f32 Rail::calcNearestRailPosCoord(const sead::Vector3f& pos, f32 interval) const {
    f32 tmp;
    return calcNearestRailPosCoord(pos, interval, &tmp);
}

// NON_MATCHING: direct asm is instruction/register identical apart from the relocated max-number constant address; tools/check reports a fixed-address mismatch.
f32 Rail::calcNearestRailPosCoord(const sead::Vector3f& pos, f32 interval, f32* distance) const {
    *distance = sead::Mathf::maxNumber();
    f32 bestParam = sead::Mathf::maxNumber();

    s32 curr_index = 0LL;
    s32 bestIndex = 0;
    for (s64 i = 0; i < mRailPartCount; i++) {
        RailPart* part = &mRailPart[curr_index];
        f32 param;
        f32 length = part->calcNearestLength(&param, pos, part->getPartLength(), interval);
        if (length < *distance) {
            *distance = length;
            bestParam = param;
            bestIndex = i;
        }
        ++curr_index;
    }

    if (bestIndex > 0)
        bestParam = bestParam + mRailPart[bestIndex - 1].getTotalDistance();
    return bestParam;
}

// NON_MATCHING: body size/register sequence is exact; tools/check first differs at the inherited anonymous-literal/call relocation (target 0x9c2c10).
f32 Rail::calcNearestRailPos(sead::Vector3f* rail_pos, const sead::Vector3f& pos,
                             f32 interval) const {
    f32 coord = calcNearestRailPosCoord(pos, interval);
    const RailPart* part = nullptr;
    f32 partDistance = 0;
    getIncludedSection(&part, &partDistance, coord);
    part->calcPos(rail_pos, part->calcCurveParam(partDistance));
    return coord;
}

bool Rail::isNearRailPoint(f32 distance, f32 epsilon) const {
    const RailPart* part = nullptr;
    f32 partDistance;
    getIncludedSection(&part, &partDistance, distance);

    return (partDistance < epsilon) || ((part->getPartLength() - partDistance) < epsilon);
}

s32 Rail::calcRailPointNum(f32 distance1, f32 distance2) const {
    if ((distance2 - distance1) < 0.01f)
        return 0;
    const RailPart* part1 = nullptr;
    const RailPart* part2 = nullptr;
    f32 partDistance1, partDistance2;
    s32 sec1 = getIncludedSection(&part1, &partDistance1, distance1);
    s32 sec2 = getIncludedSection(&part2, &partDistance2, distance2);

    return ((sec2 - sec1) + (partDistance1 < 0.01f)) +
           ((part2->getPartLength() - partDistance2) < 0.01f);
}

f32 Rail::getIncludedSectionLength(f32* partDistance, f32* length, f32 distance) const {
    const RailPart* part = nullptr;
    getIncludedSection(&part, partDistance, distance);
    f32 partLength = part->getPartLength();
    if (partDistance && length)
        *length = partLength - *partDistance;
    return partLength;
}

s32 Rail::getIncludedSectionIndex(f32 distance) const {
    return getIncludedSection(nullptr, nullptr, distance);
}

bool Rail::isIncludeBezierRailPart() const {
    for (s32 i = 0; i < mRailPartCount; i++)
        if (isBezierRailPart(i))
            return true;
    return false;
}

bool Rail::isBezierRailPart(s32 index) const {
    return mRailPart[index].isBezierCurve();
}

}  // namespace al
