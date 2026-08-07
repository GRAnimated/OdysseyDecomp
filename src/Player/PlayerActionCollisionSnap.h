#pragma once

#include <basis/seadTypes.h>
#include <cstring>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class CollisionParts;
class HitSensor;
class LiveActor;
class MtxConnector;
}  // namespace al

class IUsePlayerCollision;

class PlayerActionCollisionSnap {
public:
    PlayerActionCollisionSnap(al::LiveActor* actor, IUsePlayerCollision* collision);

    void setup(const al::CollisionParts* parts, const sead::Vector3f& position,
               const sead::Vector3f& front, const sead::Vector3f& up);
    void makeSnapPose(sead::Matrix34f* outMtx);
    void start();
    void startCommon();
    void startWallCatch();
    void moveSnapPos(const al::CollisionParts* parts, const sead::Vector3f& position,
                     const sead::Vector3f& front, const sead::Vector3f& up, s32 frame);
    void resetSnapPos(const sead::Vector3f& position);
    void setSnapPose(const sead::Vector3f& front, const sead::Vector3f& up);
    void updateSnapPose();
    void turnSnapFrontAxisUp(f32 degree);
    void rotateSnapPoseAxisFront(f32 degree);
    void rotateSnapPoseAxisSide(f32 degree);
    void rotateSnapPoseWithAxis(const sead::Vector3f& axis, f32 degree);
    void updateMove();
    void restartMoveCurrentMtx(s32 frame);
    void forceMoveEndNearestLeaveDir(const sead::Vector3f& leaveDir);
    void followCollision();
    void skipMove();
    void cancelMove();
    void updateInertia();
    void endFall(f32 velocity, const sead::Vector3f& move, f32 collisionOffset);
    bool isSnapPartsMoving() const;
    bool isSnapPartsValid() const;
    bool isSnapParts(const al::CollisionParts* parts) const;
    al::HitSensor* tryGetConnectedSensor() const;
    const sead::Vector3f& getSnapFront() const { return mState.snapFront; }
    s32 getMoveFrame() const { return mMoveFrame; }
    void calcFollowDir(sead::Vector3f* outDir, const sead::Vector3f& dir) const;

private:
    struct SnapState {
        explicit SnapState(sead::Matrix34f* snapMtx) {
            std::memset(this, 0, sizeof(*this));
            std::memcpy(snapMtx, &sead::Matrix34f::ident, sizeof(*snapMtx));
        }

        al::MtxConnector* mtxConnector;
        sead::Vector3f snapPos;
        sead::Vector3f snapFront;
        sead::Vector3f snapUp;
        sead::Vector3f _3c;
        sead::Vector3f _48;
        sead::Vector3f forceMovePower;
        const al::CollisionParts* snapParts;
    };

    static_assert(sizeof(SnapState) == 0x58);

    al::LiveActor* mActor;
    IUsePlayerCollision* mCollision;
    SnapState mState;
    sead::Matrix34f mSnapMtx;
    s32 mMoveFrame;
    s32 mMoveStep;
    const al::CollisionParts* mPreviousSnapParts;
    sead::Matrix34f mMoveStartMtx;
    sead::Matrix34f mMoveEndMtx;
    bool _108;
    u8 _109[7];
};

static_assert(sizeof(PlayerActionCollisionSnap) == 0x110);
