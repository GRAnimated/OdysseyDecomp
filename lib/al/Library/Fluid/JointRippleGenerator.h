#pragma once

#include <math/seadVector.h>

namespace al {
class LiveActor;

class JointRippleGenerator {
public:
    JointRippleGenerator(const al::LiveActor* parentActor);
    void reset();
    void updateAndGenerate();

    void setJoint(const char* joint) { mJoint = joint; }

    void setOffset(const sead::Vector3f& offset) { mOffset = offset; }

    void setOffsetX(f32 x) { mOffset.x = x; }

    void setOffsetY(f32 y) { mOffset.y = y; }

    void setOffsetZ(f32 z) { mOffset.z = z; }

    void set_2c(f32 val) { _2c = val; }

    void set_30(f32 val) { _30 = val; }

    void set_34(f32 val) { _34 = val; }

    void set_38(f32 val) { _38 = val; }

    void setData(const char* jointName, const sead::Vector3f& offset, f32 a3, f32 a4, f32 a5,
                 f32 a6) {
        mJoint = jointName;
        mOffset = offset;
        _2c = a3;
        _30 = a4;
        _34 = a5;
        _38 = a6;
    }

private:
    const al::LiveActor* mParent;
    sead::Vector3f field_8;
    const char* mJoint;
    sead::Vector3f mOffset;
    f32 _2c;
    f32 _30;
    f32 _34;
    f32 _38;
};

}  // namespace al
