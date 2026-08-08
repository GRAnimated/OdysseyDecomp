#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {

class JoyPadAccelPoseAnalyzer {
public:
    class HistoryInfo {
    public:
        HistoryInfo(s32 historyCount);
        void calcHistory(const sead::Vector3f&, const sead::Vector3f&, f32);

    private:
        u8 _0[0x90];
    };

    class PoseAxisDir {
    public:
        PoseAxisDir(s32 historyCount);
        void calcHistory(const sead::Vector3f&, const sead::Vector3f&);

    private:
        u8 _0[0x98];
    };

    JoyPadAccelPoseAnalyzer();

    void update();
    void setSwingBorder(f32, f32);
    bool isSwingLeftHand() const;
    bool isSwingRightHand() const;
    bool isSwingAnyHand() const;
    bool isSwingDoubleHand() const;
    bool isSwingDoubleHandSameDir() const;
    bool isSwingDoubleHandReverseDir() const;
    bool isSwingDoubleHandReverseInsideDir() const;
    bool isSwingDoubleHandReverseOutsideDir() const;
    const sead::Vector2f& getSwingDirDoubleHandSameDir() const;

    const sead::Vector2f& getSwingLeftHandDir() const { return mSwingDirLeftHand; }
    const sead::Vector2f& getSwingRightHandDir() const { return mSwingDirRightHand; }
    const sead::Vector2f& getSwingVelLeftHand() const { return mSwingVelLeftHand; }
    const sead::Vector2f& getSwingVelRightHand() const { return mSwingVelRightHand; }
    f32 getPoseRotZDegreeLeft() const { return mPoseRotZDegree.x; }
    f32 getPoseRotZDegreeRight() const { return mPoseRotZDegree.y; }

private:
    s32 mControllerPort;
    s32 mAccelDeviceNum;
    bool _8;
    bool mIsSwingLeft;
    bool mIsSwingRight;
    bool mIsSwingAny;
    sead::Vector2f mSwingBorder;
    sead::Vector2f mSwingDirLeftHand;
    sead::Vector2f mSwingDirRightHand;
    sead::Vector2f mSwingDirDoubleHandSameDir;
    sead::Vector2f mSwingVelLeftHand;
    sead::Vector2f mSwingVelRightHand;
    sead::Vector2f mPoseRotZDegree;
    u8 _44[4];
    HistoryInfo mHistoryLeft;
    HistoryInfo mHistoryRight;
    bool _168;
    u8 _169[7];
    PoseAxisDir _170;
    PoseAxisDir _208;
    PoseAxisDir _2a0;
    s32 _338;
    s32 _33c;
};

static_assert(sizeof(JoyPadAccelPoseAnalyzer) == 0x340);

}  // namespace al
