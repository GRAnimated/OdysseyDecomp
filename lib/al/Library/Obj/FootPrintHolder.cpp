#include "Library/Obj/FootPrintHolder.h"

#include <basis/seadNew.h>
#include <prim/seadSafeString.h>

#include "Library/Base/StringUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/Obj/FootPrintServer.h"
#include "Library/Player/PlayerUtil.h"
#include "Library/Resource/ResourceFunction.h"
#include "Library/Yaml/ByamlIter.h"
#include "Project/Obj/FootPrint.h"

namespace al {

FootPrintHolder::FootPrintHolder(LiveActor* actor, const char* objectName, HitSensor* sensor,
                                 FootPrintServer* server)
    : mServer(server), mFootPrintQueue(nullptr), mActor(actor), mSensor(sensor), mInfoIter(nullptr),
      mActionCount(0), mActionCountCopy(0), mActions(nullptr), mActionName(nullptr),
      mActionFrame(0.0f),
      mCharacterName(nullptr), mMetamorphosisName(nullptr) {
    const sead::SafeString objectNameString(objectName);
    const sead::SafeString categoryName("FootPrintInfo");
    const auto* data = tryGetBymlFromObjectResource(objectNameString, categoryName);
    if (data)
        mInfoIter = new ByamlIter(data);

    createActionList();

    mFootPrintQueue = new sead::RingBuffer<FootPrint*>;
    mFootPrintQueue->allocBuffer(mServer->_8->capacity(), nullptr);
}

void FootPrintHolder::createActionList() {
    mActionCount = mInfoIter->getSize();
    mActionCountCopy = mInfoIter->getSize();
    mActions = new Action*[mActionCount];

    for (u32 i = 0; i < static_cast<u32>(mActionCountCopy); i++) {
        mActions[i] = new Action;
        ByamlIter actionIter;
        if (mInfoIter->tryGetIterByIndex(&actionIter, i)) {
            const char* actionName;
            if (actionIter.tryGetStringByKey(&actionName, "ActionName")) {
                mActions[i]->name = actionName;
                createTimingList(mActions[i], &actionIter);
            }
        }
    }
}

void FootPrintHolder::clearAllFootPrint() {
    const s32 size = mFootPrintQueue->size();
    for (s32 i = 0; i < size; i++) {
        FootPrint* footPrint = (*mFootPrintQueue)(i);
        if (!isDead(footPrint))
            footPrint->kill();
    }
    mFootPrintQueue->clear();
}

// NON_MATCHING: exact size 700/700; target keeps the RingBuffer::remove index check and reuses the initial frame compare where current drops one CCMP and repeats one FCMP. Next source-level hypothesis: recover the descending removal-loop and positive frame-crossing condition shapes.
void FootPrintHolder::update() {
    while (mFootPrintQueue->size() >= 1 && isDead(mFootPrintQueue->front()))
        mFootPrintQueue->popFront();

    for (s32 i = mFootPrintQueue->size(); i >= 1; i--) {
        if (isDead((*mFootPrintQueue)(i - 1)))
            mFootPrintQueue->remove(i - 1);
    }

    const char* actionName = getActionName(mActor);
    if (!actionName)
        return;

    if (!mActionName || !isEqualString(mActionName, actionName))
        mActionFrame = 0.0f;

    const f32 actionFrame = getActionFrame(mActor);
    for (u32 actionIndex = 0; actionIndex < static_cast<u32>(mActionCountCopy); actionIndex++) {
        if (!isEqualString(actionName, mActions[actionIndex]->name))
            continue;
        if (mActions[actionIndex]->timingCountCopy == 0)
            continue;

        for (u32 timingIndex = 0;
             timingIndex < static_cast<u32>(mActions[actionIndex]->timingCountCopy);
             timingIndex++) {
            const f32 previousFrame = mActionFrame;
            if (previousFrame == actionFrame)
                continue;

            const f32 timingFrame =
                static_cast<f32>(mActions[actionIndex]->timings[timingIndex].frame);
            if (previousFrame > actionFrame) {
                if (previousFrame > timingFrame && timingFrame >= actionFrame)
                    continue;
            } else {
                if (previousFrame > timingFrame || timingFrame >= actionFrame)
                    continue;
            }

            appearFootPrint(mActions[actionIndex]->timings[timingIndex].offset);
            if (mServer->_8->capacity() - 20 < mFootPrintQueue->size() &&
                mFootPrintQueue->size() >= 1) {
                for (s32 i = 0; i < mFootPrintQueue->size(); i++) {
                    FootPrint* footPrint = (*mFootPrintQueue)(i);
                    if (!footPrint->isDisappear()) {
                        footPrint->startDisappear();
                        break;
                    }
                }
            }
        }
    }

    mActionName = actionName;
    mActionFrame = actionFrame;
}

FootPrint* FootPrintHolder::findDeadFootPrint() {
    return mServer->findDeadFootPrint();
}

FootPrint* FootPrintHolder::findDeadFootPrintByForce() {
    FootPrint* footPrint;
    if (mFootPrintQueue->size() <= 0)
        footPrint = mFootPrintQueue->data()[0];
    else
        footPrint = mFootPrintQueue->popFront();
    footPrint->makeActorDead();
    return footPrint;
}

const char* FootPrintHolder::getCharacterName() const {
    return mCharacterName;
}

const char* FootPrintHolder::getMetamorphosisName() const {
    return mMetamorphosisName;
}

void FootPrintHolder::createTimingList(Action* action, ByamlIter* actionIter) {
    ByamlIter timingIter;
    if (!actionIter->tryGetIterByKey(&timingIter, "Timing"))
        return;

    const s32 timingCount = timingIter.getSize();
    action->timingCount = timingCount;
    action->timingCountCopy = timingCount;
    action->timings = new Timing[timingCount];

    if (timingCount != 0) {
        for (s32 i = 0; i != timingCount; i++) {
            ByamlIter entryIter;
            if (timingIter.tryGetIterByIndex(&entryIter, i)) {
                if (!entryIter.tryGetIntByKey(&action->timings[i].frame, "Frame"))
                    action->timings[i].frame = 0;
                if (!entryIter.tryGetFloatByKey(&action->timings[i].offset.x, "OffsetX"))
                    action->timings[i].offset.x = 0.0f;
                if (!entryIter.tryGetFloatByKey(&action->timings[i].offset.y, "OffsetY"))
                    action->timings[i].offset.y = 0.0f;
                if (!entryIter.tryGetFloatByKey(&action->timings[i].offset.z, "OffsetZ"))
                    action->timings[i].offset.z = 0.0f;
            }
        }
    }
}

s32 FootPrintHolder::calcMaxAppearNum() const {
    const s32 alivePlayerNum = getAlivePlayerNum(mActor);
    const s32 maxAppearNum = alivePlayerNum == 2 ? 8 : 5;
    return alivePlayerNum < 2 ? 12 : maxAppearNum;
}

}  // namespace al
