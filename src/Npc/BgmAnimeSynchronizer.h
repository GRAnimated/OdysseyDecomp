#pragma once

#include <basis/seadTypes.h>

namespace al {
class ByamlIter;
class LiveActor;
}  // namespace al

class BgmAnimeSynchronizer {
public:
    static BgmAnimeSynchronizer* tryCreate(al::LiveActor*, const al::ByamlIter&);
    void trySyncBgm();
    void setSyncChaseRateOffsetMax(f32, f32, f32, f32);

    const char* _0;
    al::LiveActor* mActor;
    s32 _10;
    bool _14;
    u8 _15[3];
    u8 _18[16];
    bool _28;
};
