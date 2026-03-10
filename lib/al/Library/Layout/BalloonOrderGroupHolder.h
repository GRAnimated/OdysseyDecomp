#pragma once

#include <math/seadVector.h>

namespace al {

class BalloonOrderGroup;
class PlacementInfo;

class BalloonOrderGroupHolder {
public:
    BalloonOrderGroupHolder();
    const BalloonOrderGroup* tryFindGroup(const PlacementInfo& info) const;
    void registerGroup(BalloonOrderGroup* group);
    void updateAll(const sead::Vector3f& playerPos);
    void resetInsideTerritoryAll();

private:
    void* _0;
    void* _8;
};

}  // namespace al
