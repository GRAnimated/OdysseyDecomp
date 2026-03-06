#include "Npc/SphinxQuiz.h"

#include <math/seadQuat.h>
#include <math/seadVector.h>

#include "Library/Base/StringUtil.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/Event/EventFlowFunction.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/KeyPose/KeyPoseKeeperUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAreaFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementInfo.h"
#include "Library/Stage/StageSwitchUtil.h"

#include "Npc/SphinxQuizRouteKillExecutor.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "System/GameDataUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerDemoUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(SphinxQuiz, Wait);
NERVE_IMPL(SphinxQuiz, WaitMiniGame);
NERVE_IMPL(SphinxQuiz, KeyWait);
NERVE_IMPL(SphinxQuiz, KeyMove);

NERVES_MAKE_NOSTRUCT(SphinxQuiz, KeyMove);
NERVES_MAKE_STRUCT(SphinxQuiz, Wait, WaitMiniGame, KeyWait);

const char* sWorldNames[] = {"Sand", "Forest", "Sea", "Moon"};

const sead::Vector3f sHintOffset = {0.0f, 0.0f, 900.0f};
const sead::Vector3f sWatchOffset = {0.0f, 775.0f, 550.0f};
const sead::Vector3f sCameraOffset = {0.0f, 175.0f, 750.0f};
}  // namespace

// NON_MATCHING: WFixedSafeString inlining difference — target has buffer-check + memset path
SphinxQuiz::SphinxQuiz(const char* name) : al::LiveActor(name) {}

// NON_MATCHING: stack layout and instruction scheduling around calcSlerpKeyQuat/calcLerpKeyTrans
void SphinxQuiz::init(const al::ActorInitInfo& info) {
    al::initActor(this, info);
    al::initNerve(this, &NrvSphinxQuiz.Wait, 1);

    if (rs::isSequenceTimeBalloonOrRace(this)) {
        al::setNerve(this, &NrvSphinxQuiz.WaitMiniGame);
        makeActorAlive();
        return;
    }

    al::tryGetStringArg(&mQuizType, info, "QuizType");
    mEventFlowExecutor = rs::initEventFlow(this, info, "SphinxQuiz", mQuizType);
    al::initEventReceiver(mEventFlowExecutor, this);
    rs::initEventCharacterName(mEventFlowExecutor, info, "SphinxQuiz");
    rs::makeEventCharacterName(&mEventCharacterName, info, "GateKeepersUnknown");

    if (rs::isDefinedEventCamera(mEventFlowExecutor, "Quiz")) {
        rs::initEventCameraFixActorAutoAroundFront(mEventFlowExecutor, info, "Quiz", this,
                                                   &sCameraOffset, 2450.0f, 40.0f, 15.0f, true);
    }

    rs::startEventFlow(mEventFlowExecutor, "Wait");

    al::PlacementInfo linkInfo;
    al::getLinksInfo(&linkInfo, info, "KeyMoveNext");
    mKeyMoveSaveObjInfo = rs::createSaveObjInfoWriteSaveData(info, linkInfo);
    mSaveObjInfo = rs::createSaveObjInfoWriteSaveData(info);
    mKeyPoseKeeper = al::createKeyPoseKeeper(info);
    mRouteKillAreaGroup = al::createLinkAreaGroup(
        this, info, "RouteKillArea",
        u8"ルートアクター消滅エリアグループ",
        u8"ルートアクター消滅エリア");

    if (mRouteKillAreaGroup)
        rs::tryCreateSphinxQuizRouteKillExecutor(this);

    if (al::isExistLinkChild(info, "ShineActor", 0))
        mShineIndex = GameDataFunction::tryFindLinkedShineIndexByLinkName(this, info, "ShineActor");

    if (rs::isOnSaveObjInfo(mSaveObjInfo)) {
        sead::Quatf quat = sead::Quatf::unit;
        sead::Vector3f trans(0.0f, 0.0f, 0.0f);
        al::KeyPoseKeeper* keeper = mKeyPoseKeeper;
        al::calcSlerpKeyQuat(&quat, keeper, 1.0f);
        al::calcLerpKeyTrans(&trans, keeper, 1.0f);
        al::resetQuatPosition(this, quat, trans);
        rs::startEventFlow(mEventFlowExecutor, "AfterAll");
        makeActorAlive();
        return;
    }

    if (rs::isOnSaveObjInfo(mKeyMoveSaveObjInfo)) {
        sead::Quatf quat = sead::Quatf::unit;
        sead::Vector3f trans(0.0f, 0.0f, 0.0f);
        al::KeyPoseKeeper* keeper = mKeyPoseKeeper;
        al::calcSlerpKeyQuat(&quat, keeper, 1.0f);
        al::calcLerpKeyTrans(&trans, keeper, 1.0f);
        al::resetQuatPosition(this, quat, trans);
        rs::startEventFlow(mEventFlowExecutor, "AfterKeyMove");
        makeActorAlive();
        return;
    }

    makeActorAlive();
}

bool SphinxQuiz::receiveMsg(const al::SensorMsg* msg, al::HitSensor* self, al::HitSensor* other) {
    return rs::isMsgPlayerDisregardHomingAttack(msg);
}

// NON_MATCHING: compiler tail-merges JudgeAfterCorrect handlers via CSE; our code keeps them separate
bool SphinxQuiz::receiveEvent(const al::EventFlowEventData* event) {
    if (al::isEventName(event, "KeyMove")) {
        if (rs::isOnSaveObjInfo(mKeyMoveSaveObjInfo))
            return true;
        if (al::isNerve(this, &NrvSphinxQuiz.Wait))
            al::setNerve(this, &NrvSphinxQuiz.KeyWait);
        return false;
    }

    if (al::isEventName(event, "RecordSuccess")) {
        s32 counter = mRecordCounter;
        if (counter < 0) {
            al::startHitReaction(this, u8"正解");
            counter = 45;
            mRecordCounter = 45;
        }
        bool result = counter < 2;
        if (counter < 2)
            mRecordCounter = -1;
        else
            mRecordCounter = counter - 1;
        return result;
    }

    if (al::isEventName(event, "RecordFailed")) {
        s32 counter = mRecordCounter;
        if (counter < 0) {
            al::startHitReaction(this, u8"不正解");
            counter = 45;
            mRecordCounter = 45;
            mHasFailed = true;
        }
        bool result = counter < 2;
        if (counter < 2)
            mRecordCounter = -1;
        else
            mRecordCounter = counter - 1;
        return result;
    }

    if (al::isEventName(event, "RecordCorrectAll")) {
        rs::onSaveObjInfo(mSaveObjInfo);
        rs::answerCorrectSphinxQuizAll(this);
        al::tryOnStageSwitch(this, "SwitchCompleteOn");
        return true;
    }

    if (al::isEventName(event, "JudgeContinuationCorrect"))
        return !mHasFailed;

    if (al::isEventName(event, "JudgeAfterCorrectSelect")) {
        for (s32 i = 0; i < 4; i++) {
            const char* worldName = sWorldNames[i];
            al::StringTmp<32> param1st("Is%s1st", worldName);
            if (al::isEventDataParamBool(event, param1st.cstr())) {
                GameDataHolderAccessor accessor(this);
                if (!rs::isAnswerCorrectSphinxQuiz(accessor, worldName))
                    return false;
            }
            if (!al::isEqualString(worldName, "Forest")) {
                al::StringTmp<32> paramAll("Is%sAll", worldName);
                if (al::isEventDataParamBool(event, paramAll.cstr())) {
                    GameDataHolderAccessor accessor(this);
                    if (!rs::isAnswerCorrectSphinxQuizAll(accessor, worldName))
                        return false;
                }
            }
        }
        return true;
    }

    const char* worldName = sWorldNames[0];
    if (al::isEventName(event, "JudgeAfterCorrect%s", "Sand"))
        return rs::isAnswerCorrectSphinxQuiz(GameDataHolderAccessor(this), worldName);

    if (al::isEventName(event, "JudgeAfterCorrectAll%s", "Sand"))
        return rs::isAnswerCorrectSphinxQuizAll(GameDataHolderAccessor(this), worldName);

    worldName = sWorldNames[1];
    if (al::isEventName(event, "JudgeAfterCorrect%s", "Forest"))
        return rs::isAnswerCorrectSphinxQuiz(GameDataHolderAccessor(this), worldName);

    if (al::isEventName(event, "JudgeAfterCorrectAll%s", "Forest"))
        return rs::isAnswerCorrectSphinxQuizAll(GameDataHolderAccessor(this), worldName);

    worldName = sWorldNames[2];
    if (al::isEventName(event, "JudgeAfterCorrect%s", "Sea"))
        return rs::isAnswerCorrectSphinxQuiz(GameDataHolderAccessor(this), worldName);

    if (al::isEventName(event, "JudgeAfterCorrectAll%s", "Sea"))
        return rs::isAnswerCorrectSphinxQuizAll(GameDataHolderAccessor(this), worldName);

    worldName = sWorldNames[3];
    if (al::isEventName(event, "JudgeAfterCorrect%s", "Moon"))
        return rs::isAnswerCorrectSphinxQuiz(GameDataHolderAccessor(this), worldName);

    if (al::isEventName(event, "JudgeAfterCorrectAll%s", "Moon"))
        return rs::isAnswerCorrectSphinxQuizAll(GameDataHolderAccessor(this), worldName);

    if (al::isEventName(event, "SetCharacterNameUnknown")) {
        rs::swapEventCharacterName(mEventFlowExecutor, &mEventCharacterName);
        return true;
    }

    if (al::isEventName(event, "ResetCharacterName")) {
        rs::resetEventCharacterName(mEventFlowExecutor);
        return true;
    }

    if (al::isEventName(event, "KillAllActorsIfOnRoute")) {
        if (mRouteKillAreaGroup) {
            PlayerDemoFunction::prepareSphinxQuizRouteKill(this);
            al::HitSensor* bodySensor = al::getHitSensor(this, "Body");
            rs::sendMsgSphinxQuizRouteKill(bodySensor, mRouteKillAreaGroup);
        }
        return true;
    }

    return false;
}

void SphinxQuiz::exeWait() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Wait");
        s32 shineIndex = mShineIndex;
        if (shineIndex >= 0) {
            sead::Vector3f hintTrans(0.0f, 0.0f, 0.0f);
            al::calcTransLocalOffset(&hintTrans, this, sHintOffset);
            GameDataFunction::setHintTrans(GameDataHolderAccessor(this), shineIndex, hintTrans);
        }
    }

    rs::updateEventFlow(mEventFlowExecutor);

    if (rs::isActiveEventDemo(this) && rs::isEqualEventDemoStartActor(this)) {
        sead::Vector3f watchTrans(0.0f, 0.0f, 0.0f);
        al::calcTransLocalOffset(&watchTrans, this, sWatchOffset);
        rs::validateWatchTarget(this, watchTrans);
    }
}

void SphinxQuiz::exeKeyWait() {
    if (al::isFirstStep(this))
        mKeyWaitTime = al::calcKeyMoveWaitTime(mKeyPoseKeeper);

    if (rs::isActiveEventDemo(this) && rs::isEqualEventDemoStartActor(this)) {
        sead::Vector3f watchTrans(0.0f, 0.0f, 0.0f);
        al::calcTransLocalOffset(&watchTrans, this, sWatchOffset);
        rs::validateWatchTarget(this, watchTrans);
    }

    al::setNerveAtGreaterEqualStep(this, &KeyMove, mKeyWaitTime);
}

void SphinxQuiz::exeKeyMove() {
    s32 moveTime;

    if (al::isFirstStep(this)) {
        const char* quizType = mQuizType;
        const char* actionName;
        if (al::isInWater(this))
            actionName = "MoveWater";
        else if (al::isEqualString(quizType, "QuizForest"))
            actionName = "MoveForest";
        else if (al::isEqualString(quizType, "QuizHackParadeSavage"))
            actionName = "MoveHackParadeSavage";
        else if (al::isStartWithString(quizType, "QuizMoon"))
            actionName = "MoveMoon";
        else
            actionName = "MoveGround";
        al::startAction(this, actionName);
        moveTime = al::calcKeyMoveMoveTime(mKeyPoseKeeper);
        mKeyMoveTime = moveTime;
    } else {
        moveTime = mKeyMoveTime;
    }

    f32 rate = al::calcNerveRate(this, moveTime);
    al::calcLerpKeyTrans(al::getTransPtr(this), mKeyPoseKeeper, rate);
    al::calcSlerpKeyQuat(al::getQuatPtr(this), mKeyPoseKeeper, rate);
    al::requestCameraLoopShakeWeak(this);

    if (mShineIndex >= 0) {
        s32 shineIndex = mShineIndex;
        sead::Vector3f hintTrans(0.0f, 0.0f, 0.0f);
        al::calcTransLocalOffset(&hintTrans, this, sHintOffset);
        GameDataFunction::setHintTrans(GameDataHolderAccessor(this), shineIndex, hintTrans);
    }

    if (rs::isActiveEventDemo(this) && rs::isEqualEventDemoStartActor(this)) {
        sead::Vector3f watchTrans(0.0f, 0.0f, 0.0f);
        al::calcTransLocalOffset(&watchTrans, this, sWatchOffset);
        rs::validateWatchTarget(this, watchTrans);
    }

    if (al::isGreaterEqualStep(this, mKeyMoveTime)) {
        al::nextKeyPose(mKeyPoseKeeper);
        if (al::isStop(mKeyPoseKeeper)) {
            al::startHitReaction(this, u8"停止");
            rs::onSaveObjInfo(mKeyMoveSaveObjInfo);
            rs::answerCorrectSphinxQuiz(this);
            al::setNerve(this, &NrvSphinxQuiz.Wait);
        } else {
            al::setNerve(this, &NrvSphinxQuiz.KeyWait);
        }
    }
}

void SphinxQuiz::exeWaitMiniGame() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Wait");
        s32 shineIndex = mShineIndex;
        if (shineIndex >= 0) {
            sead::Vector3f hintTrans(0.0f, 0.0f, 0.0f);
            al::calcTransLocalOffset(&hintTrans, this, sHintOffset);
            GameDataFunction::setHintTrans(GameDataHolderAccessor(this), shineIndex, hintTrans);
        }
    }
}
