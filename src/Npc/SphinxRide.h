#pragma once

#include <basis/seadTypes.h>

#include "Library/Event/IEventFlowEventReceiver.h"
#include "Library/Event/IEventFlowQueryJudge.h"
#include "Library/LiveActor/LiveActor.h"

class IUsePlayerPuppet;

class SphinxRide : public al::LiveActor,
                   public al::IEventFlowEventReceiver,
                   public al::IEventFlowQueryJudge {
public:
    SphinxRide(const char* name);

    bool receiveEvent(const al::EventFlowEventData* event) override;
    const char* judgeQuery(const char* query) const override;

    bool tryGetOffAndCancelBind();
    void controlAndRotateYAndMove(f32 rotateAngle, f32 speedRatio, bool notWallCollided);
    void updateGravityAndDump(f32 gravityAngle, f32 dumpRatio);
    void trySlipOnMoveLimit();
    bool isPlayerInputSwingSphinxStop() const;
    bool isValidateCameraAngleV(f32 angle);
    void disappear();
    void setupEventFade();
    void startEventWait();
    void endEventWait(f32 blendFrame);

    IUsePlayerPuppet* mPlayerPuppet = nullptr;
    void* _120 = nullptr;
    void* _128 = nullptr;
    void* _130 = nullptr;
    void* _138 = nullptr;
    void* _140 = nullptr;
    void* _148 = nullptr;
    void* _150 = nullptr;
    f32 mCameraAngle = 0;
    f32 mStartDegree = 0;
};
