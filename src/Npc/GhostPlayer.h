#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadEnum.h>
#include <prim/seadSafeString.h>

#include "Library/LiveActor/LiveActor.h"

namespace al {
struct ActorInitInfo;
class ByamlIter;
class ParabolicPath;
class WaterSurfaceFinder;
}  // namespace al

class RaceManShell;

class GhostPlayer : public al::LiveActor {
public:
    SEAD_ENUM(GhostPlayerColor, RaceManRed, RaceManBlue, RaceManGreen, RaceManViolet, RaceManGold)

    GhostPlayer(const char* name, s32 goalIndex, s32 color);

    void init(const al::ActorInitInfo& info) override;
    void initAfterPlacement() override;
    void attackSensor(al::HitSensor* self, al::HitSensor* other) override;
    bool receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) override;
    void control() override;

    void initCommon(const al::ActorInitInfo& info);
    void initWithArchiveName(const al::ActorInitInfo& info, const char* archiveName,
                             const char* suffix);
    void applyGhostData(s32 step);
    bool initGhostPlayDataFromByaml(const al::ByamlIter* iter);
    bool isPlayDone() const;
    bool initGhostPlayDataFromByaml(const char* name);
    void initThrowCap(al::LiveActor* cap);
    void start();
    void exeWait();
    void showDefault();
    void exePlay();
    void attachJumpToTarget(const sead::Vector3f& target);
    void exeAttachJump();
    void exeEnd();
    void exeResult();
    void exeResultLose();
    void exeResultWin();

    s32 mGoalIndex;
    GhostPlayerColor mColor;
    s32 mRaceManStep = 0;
    sead::FixedSafeString<64> mActionName;
    s32 mDataArraySize = 0;
    const al::ByamlIter* mPlayDataIter = nullptr;
    s32 mActionFrame = 0;
    al::ParabolicPath* mParabolicPath = nullptr;
    void* _190 = nullptr;
    s32 _198 = 0;
    bool _19c = false;
    bool _19d = false;
    bool _19e = false;
    sead::Vector3f _1a0 = {0, 0, 0};
    al::LiveActor* mCapActor = nullptr;
    RaceManShell* mShell = nullptr;
    s32 mHackStartFrame = 0;
    s32 mHackEndFrame = -1;
    bool mIsHackActive = false;
    s32 _1cc = 0;
    sead::Vector3f _1D0 = {0, 0, 0};
    sead::Matrix34f mWaterSurfaceMtx = sead::Matrix34f::ident;
    al::WaterSurfaceFinder* mWaterSurfaceFinder = nullptr;
    bool mIsInWater = false;
    sead::Vector3f mPrevTrans = {0, 0, 0};
    bool _228 = false;
    bool _229 = false;
};

static_assert(sizeof(GhostPlayer) == 0x230);
