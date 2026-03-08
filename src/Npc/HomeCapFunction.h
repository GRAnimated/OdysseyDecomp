#pragma once

namespace sead {
template <typename T>
struct Vector3;
using Vector3f = Vector3<f32>;
}  // namespace sead

namespace HomeCapFunction {
const sead::Vector3f& getAimChairOffset();
}  // namespace HomeCapFunction
