#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveExecutor.h"

namespace al {
class LiveActor;
}

struct PlayerRecordEntry {
    const char* animName;
    sead::Vector3f trans;
    sead::Quatf posture;
    bool hasCapPos;
    sead::Vector3f capPos;
};

static_assert(sizeof(PlayerRecordEntry) == 0x38);

class PlayerRecoder : public al::NerveExecutor {
public:
    PlayerRecoder(const al::LiveActor* actor, s32 maxFrames);
    ~PlayerRecoder() override;

    void start();
    void update();
    void endRecord();

    const sead::Vector3f& getTrans(s32 index) const;
    const sead::Quatf& getPosture(s32 index) const;
    const char* getAnimName(s32 index) const;
    bool isEndRecode() const;

    void exeWait();
    void exeRecode();
    void exeEnd();

private:
    const al::LiveActor* mActor;
    PlayerRecordEntry* mEntries = nullptr;
    s32 mMaxFrames;
    s32 mCurrentFrame = 0;
};
