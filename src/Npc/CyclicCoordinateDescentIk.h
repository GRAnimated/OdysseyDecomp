#pragma once

namespace sead {
template <typename T>
struct Vector3;
using Vector3f = Vector3<f32>;
}  // namespace sead

namespace al {
class LiveActor;
}  // namespace al

class CyclicCoordinateDescentIk {
public:
    CyclicCoordinateDescentIk(al::LiveActor* actor, s32 count);
    void createConnection(const char* rootName, const char* tipName);
    void setLimitDegree(const char* boneName, const sead::Vector3f& min,
                        const sead::Vector3f& max, const char* connectionName);
    void updateEffector(const sead::Vector3f& target, f32 rate,
                        const char* connectionName);
};
