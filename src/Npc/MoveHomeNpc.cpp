#include "Npc/MoveHomeNpc.h"

#include <math/seadVector.h>

#include "Library/Joint/JointControllerKeeper.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "Util/CapManHeroDemoUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerDemoUtil.h"

namespace {
NERVE_IMPL(MoveHomeNpc, Talk);
NERVE_IMPL(MoveHomeNpc, TalkEnd);
NERVES_MAKE_NOSTRUCT(MoveHomeNpc, Talk, TalkEnd);
}  // namespace

MoveHomeNpc::MoveHomeNpc(const char* name) : al::LiveActor(name) {}

void MoveHomeNpc::init(const al::ActorInitInfo& initInfo) {
    al::initActorWithArchiveName(this, initInfo, "CapManHero", "DemoHome");
    initHitSensor(1);
    sead::Vector3f offset{0.0f, 0.0f, 0.0f};
    al::addHitSensorEye(this, initInfo, "DemoSkip", 0.0f, 8, offset);
    al::initJointControllerKeeper(this, 1);
    CapManHeroDemoUtil::initCapManHeroTailJointController(this);
    mEventFlowExecutor = rs::initEventFlow(this, initInfo, nullptr, nullptr);
    al::initNerve(this, &Talk, 0);
    makeActorDead();
}

void MoveHomeNpc::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isSensorPlayer(other))
        al::sendMsgPush(other, self);
}

bool MoveHomeNpc::receiveMsg(const al::SensorMsg* msg, al::HitSensor* self, al::HitSensor* other) {
    return false;
}

void MoveHomeNpc::startTalk() {
    appear();
    al::invalidateClipping(this);
    rs::tryStartEventCutSceneDemo(this);
    rs::hideDemoPlayerSilhouette(this);
    al::setNerve(this, &Talk);
}

void MoveHomeNpc::skipTalk() {
    rs::startCloseNpcDemoEventTalkMessage(this);
    rs::endEventCutSceneDemo(this);
}

void MoveHomeNpc::exeTalk() {
    if (al::isFirstStep(this)) {
        GameDataHolderAccessor accessor(this);
        if (GameDataFunction::isForwardWorldWarpDemo(accessor)) {
            GameDataHolderAccessor accessor2(this);
            s32 nextWorld = GameDataFunction::getNextWorldId(accessor2);
            s32 sandWorld = GameDataFunction::getWorldIndexSand();
            if (nextWorld == sandWorld) {
                GameDataHolderAccessor accessor3(this);
                s32 sandIndex = GameDataFunction::getWorldIndexSand();
                if (GameDataFunction::isAlreadyGoWorld(accessor3, sandIndex)) {
                    rs::startEventFlow(mEventFlowExecutor, "TalkStart");
                } else {
                    rs::startEventFlow(mEventFlowExecutor, "TalkStartFirstTime");
                    al::hideModel(this);
                    rs::startMarioCapEyeAction(this, "DemoSitDownWaitFirst");
                }
            } else {
                rs::startEventFlow(mEventFlowExecutor, "TalkStart");
            }
        } else {
            rs::startEventFlow(mEventFlowExecutor, "TalkStart");
        }
        rs::clearDemoAnimInterpolatePlayer(this);
    }

    rs::resetMarioDynamics(this);
    if (rs::updateEventFlow(mEventFlowExecutor)) {
        rs::keepMarioCapVisibilityEndDemo(this);
        rs::endEventCutSceneDemo(this);
        al::validateClipping(this);
        al::setNerve(this, &TalkEnd);
    }
}

void MoveHomeNpc::exeTalkEnd() {}
