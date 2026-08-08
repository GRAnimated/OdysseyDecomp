#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

class PlayerColliderDisk;

namespace al {
class Triangle;

class TriangleFilterBase {
public:
    virtual bool isInvalidTriangle(const Triangle& triangle) const = 0;
};

class TriangleFilterGroundOnly : public TriangleFilterBase {
public:
    TriangleFilterGroundOnly(const sead::Vector3f& down) : mDown(down) {}

    bool isInvalidTriangle(const Triangle& triangle) const override;

private:
    const sead::Vector3f& mDown;
};

class TriangleFilterWallOnly : public TriangleFilterBase {
public:
    TriangleFilterWallOnly(const sead::Vector3f& down) : mDown(down) {}

    bool isInvalidTriangle(const Triangle& triangle) const override;

private:
    const sead::Vector3f& mDown;
};

template <typename T>
class TriangleFilterDelegator : public TriangleFilterBase {
public:
    using DelegateFilter = bool (T::*)(const Triangle&) const;

    TriangleFilterDelegator(T* parent, DelegateFilter filter) : mParent(parent), mFunc(filter) {}

    bool isInvalidTriangle(const Triangle& triangle) const override {
        return (mParent->*mFunc)(triangle);
    }

private:
    T* mParent;
    DelegateFilter mFunc;
};

class DiskInterpolator {
public:
    DiskInterpolator();

    void startInterp(const sead::Vector3f&, const sead::Vector3f&, f32, f32, f32, f32,
                     const sead::Matrix34f*, const sead::Matrix34f*, const sead::Vector3f&, f32);
    void nextStep();
    void calcInterpPos(sead::Vector3f*) const;
    void getMoveVector(sead::Vector3f*);
    void calcInterp(sead::Vector3f*, f32*, f32*, sead::Vector3f*, sead::Matrix34f*,
                    sead::Vector3f*) const;

private:
    friend class ::PlayerColliderDisk;

    u8 _0[0x48];
    f32 _48;
    f32 _4c;
};

}  // namespace al
