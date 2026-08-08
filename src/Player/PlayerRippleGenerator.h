#pragma once

#include <basis/seadTypes.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
}
class PlayerModelHolder;

class PlayerRippleGenerator {
public:
    PlayerRippleGenerator(const al::LiveActor* player, const al::LiveActor* cap,
                          const PlayerModelHolder* modelHolder);

    void reset();
    void tryDeleteReaction(bool isInWater, bool isOnGround, bool isHack);
    void updateAndGenerate(bool isInWater, bool isOnWaterSurface, bool isHipDrop,
                           bool isSeparateHipDrop);
    void setOffset(const sead::Vector3f& offset) { mOffset = offset; }

private:
    const al::LiveActor* mPlayer;
    const al::LiveActor* mCap;
    const PlayerModelHolder* mModelHolder;
    sead::Vector3f mOffset = sead::Vector3f::zero;
    sead::Vector3f mPreviousPlayerPos;
    sead::Vector3f mPreviousCapPos;
    s32 _3c;
    sead::Vector3f* mPreviousHandPositions;
    sead::Quatf* mPreviousSpineQuat;
    bool mIsCloudReaction;
    bool mIsGrassReaction;
    bool mIsFlowerReaction;
    u8 _53[5];
};

