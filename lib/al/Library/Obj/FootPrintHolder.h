#pragma once

#include <basis/seadTypes.h>
#include <container/seadRingBuffer.h>
#include <math/seadVector.h>

namespace al {
class ByamlIter;
class FootPrint;
class FootPrintServer;
class HitSensor;
class LiveActor;

class FootPrintHolder {
public:
    struct Timing {
        s32 frame;
        sead::Vector3f offset;
    };

    struct Action {
        const char* name;
        s32 timingCount;
        s32 timingCountCopy;
        Timing* timings;
    };

    FootPrintHolder(LiveActor* actor, const char* objectName, HitSensor* sensor, FootPrintServer* server);
    void createActionList();
    void clearAllFootPrint();
    void update();
    void appearFootPrint(const sead::Vector3f&);
    FootPrint* findDeadFootPrint();
    FootPrint* findDeadFootPrintByForce();
    const char* getCharacterName() const;
    const char* getMetamorphosisName() const;
    void createTimingList(Action* action, ByamlIter* actionIter);
    s32 calcMaxAppearNum() const;

    void clearAnimationNames() {
        mCharacterName = nullptr;
        mMetamorphosisName = nullptr;
    }

private:
    FootPrintServer* mServer;
    sead::RingBuffer<FootPrint*>* mFootPrintQueue;
    LiveActor* mActor;
    HitSensor* mSensor;
    ByamlIter* mInfoIter;
    s32 mActionCount;
    s32 mActionCountCopy;
    Action** mActions;
    const char* mActionName;
    f32 mActionFrame;
    u32 _44;
    const char* mCharacterName;
    const char* mMetamorphosisName;
};
static_assert(sizeof(FootPrintHolder) == 0x58);
}  // namespace al
