#pragma once

#include <basis/seadTypes.h>

namespace sead {
template <typename T>
struct Vector3;
using Vector3f = Vector3<f32>;
}  // namespace sead

namespace al {

class BalloonOrderGroupHolder {
public:
    BalloonOrderGroupHolder();
    void updateAll(const sead::Vector3f& playerPos);
    void resetInsideTerritoryAll();

private:
    void* _00;
    void* _08;
};

}  // namespace al
