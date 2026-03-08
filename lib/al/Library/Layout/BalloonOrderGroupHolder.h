#pragma once

#include <math/seadVector.h>

namespace al {

class BalloonOrderGroupHolder {
public:
    BalloonOrderGroupHolder();
    void updateAll(const sead::Vector3f& playerPos);
    void resetInsideTerritoryAll();

private:
    void* _0;
    void* _8;
};

}  // namespace al
