#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class HitSensor;
class LiveActor;

bool isExistItemKeeper(const al::LiveActor*);
void addItem(al::LiveActor*, const al::ActorInitInfo&, const char*, const char*, const char*, s32,
             bool);
void addItem(al::LiveActor*, const al::ActorInitInfo&, const char*, bool);
void setAppearItemFactor(const al::LiveActor*, const char*, const al::HitSensor*);
void setAppearItemOffset(const al::LiveActor*, const sead::Vector3f&);
void setAppearItemAttackerSensor(const al::LiveActor*, const al::HitSensor*);
void appearItem(const al::LiveActor*);
void appearItem(const al::LiveActor*, const sead::Vector3f&, const sead::Quatf&,
                const al::HitSensor*);
void appearItem(const al::LiveActor*, const sead::Vector3f&, const sead::Vector3f&,
                const al::HitSensor*);
void appearItemTiming(const al::LiveActor*, const char*);
void appearItemTiming(const al::LiveActor*, const char*, const sead::Vector3f&, const sead::Quatf&,
                      const al::HitSensor*);
void appearItemTiming(const al::LiveActor*, const char*, const sead::Vector3f&,
                      const sead::Vector3f&, const al::HitSensor*);
void acquireItem(const al::LiveActor*, al::HitSensor*, const char*);
s32 getItemType(const al::LiveActor*, const char*);
}  // namespace al