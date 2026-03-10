#pragma once

namespace sead {
template <typename T>
struct Vector3;
using Vector3f = Vector3<f32>;
}  // namespace sead

class YukimaruInput {
public:
    virtual bool isTriggerJump() const;
    virtual bool isHoldJump() const;
    virtual bool isSwingDirLeft() const;
    virtual bool isSwingDirRight() const;
    virtual void calcInputVec(sead::Vector3f* out) const;
};
