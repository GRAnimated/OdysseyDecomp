#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <math/seadVector.h>

#include "Library/Collision/IUseCollision.h"

namespace al {
struct ActorInitInfo;
class CollisionDirector;
class HitSensor;
class MtxConnector;
class SceneCameraInfo;
class ScreenPointer;
}  // namespace al

class TouchTargetInfo;

class TouchTargetKeeper : public al::IUseCollision {
public:
    TouchTargetKeeper(al::HitSensor* sensor, s32 maxStoreCount);

    void init(const al::ActorInitInfo& info);
    void update();

    bool tryGetLastHoldTouchTarget(TouchTargetInfo* targetInfo) const;
    bool tryGetLastDecideTouchTarget(TouchTargetInfo* targetInfo) const;
    void storeTouchTarget();
    void freeTouchTarget();
    void clearTouchTarget();

    bool isStoreFull() const;
    s32 getStoreTouchTargetNum() const;
    TouchTargetInfo* getStoreTouchTargetInfo(s32 index);

    al::CollisionDirector* getCollisionDirector() const override;

    bool isTouchTarget() const { return _99; }

private:
    al::HitSensor* mHitSensor = nullptr;
    sead::Vector2f _10 = {-1000.0f, -1000.0f};
    al::SceneCameraInfo* _18 = nullptr;
    al::CollisionDirector* mCollisionDirector = nullptr;
    al::ScreenPointer* _28 = nullptr;
    TouchTargetInfo* _30 = nullptr;
    TouchTargetInfo* _38 = nullptr;
    al::MtxConnector* _40 = nullptr;
    al::MtxConnector* _48 = nullptr;
    s32 _50 = 0;
    sead::PtrArray<TouchTargetInfo> _58;
    sead::PtrArray<TouchTargetInfo> _68;
    sead::PtrArray<al::MtxConnector> _78;
    sead::PtrArray<al::MtxConnector> _88;
    bool _98 = false;
    bool _99 = false;
};

