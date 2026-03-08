#include "Npc/HelpNpc.h"

#include "Library/Base/StringUtil.h"
#include "Library/Event/EventFlowEventData.h"
#include "Library/Event/EventFlowFunction.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/Layout/LayoutInitInfo.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitInfo.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Shadow/ActorShadowUtil.h"

#include "Npc/EventActorMovementTurn.h"
#include "Npc/NpcEventStateScare.h"
#include "Npc/NpcStateReaction.h"
#include "Npc/NpcStateReactionParam.h"
#include "Npc/TalkNpcActionAnimInfo.h"
#include "Scene/HtmlViewerRequester.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/SensorMsgFunction.h"

namespace {
NERVE_IMPL(HelpNpc, Wait);
NERVE_IMPL(HelpNpc, Scare);
NERVE_IMPL(HelpNpc, Reaction);
NERVES_MAKE_NOSTRUCT(HelpNpc, Wait, Scare, Reaction);

NpcStateReactionParam sReactionParam("Reaction", "ReactionCap");
NpcEventStateScareActionParam sScareParam("Scared");
}  // namespace

HelpNpc::HelpNpc(const char* name) : al::LiveActor(name) {}

void HelpNpc::init(const al::ActorInitInfo& info) {
    al::initMapPartsActor(this, info, nullptr);
    al::initNerve(this, &Wait, 3);

    mTalkNpcParam = rs::initTalkNpcParam(this, nullptr);
    auto* animInfo = new TalkNpcActionAnimInfo();
    mActionAnimInfo = animInfo;
    animInfo->mWaitActionName = "Wait";
    animInfo->init(this, info, mTalkNpcParam, nullptr);

    const char* modelName = al::getModelName(this);

    if (al::isEqualString(modelName, "ForestMan") || al::isEqualString(modelName, "DesertMan") ||
        al::isEqualString(modelName, "CityMan")) {
        al::startVisAnim(this, "Off");
    }

    if (al::isEqualString(modelName, "ForestMan") || al::isEqualString(modelName, "CityMan"))
        mIsCapTarget = true;

    bool isCityMan = false;
    if (al::isEqualString(modelName, "CityMan")) {
        al::startMtpAnim(this, "ClothBlack");
        al::startMclAnim(this, "SkinYellow");
        isCityMan = true;
    }

    bool isCapMan = false;
    if (al::isEqualString(modelName, "CapMan")) {
        al::startMclAnim(this, "Brown");
        isCapMan = true;
    }

    if (al::isEqualString(modelName, "ForestMan"))
        al::startMtpAnim(this, "ForestManB");

    f32 shadowDropLength = -1.0f;
    if (al::tryGetArg(&shadowDropLength, info, "ShadowDropLength") && shadowDropLength >= 0.0f) {
        al::isExistShadowMask(this, u8"体影");
        al::setShadowMaskDropLength(this, shadowDropLength, u8"体影");
        al::expandClippingRadiusByShadowLength(this, nullptr, shadowDropLength);
    }

    f32 shadowIntensity = -1.0f;
    if (al::tryGetArg(&shadowIntensity, info, "ShadowIntensity") && shadowIntensity >= 0.0f) {
        al::isExistShadowMask(this, u8"体影");
        al::setShadowMaskIntensity(this, u8"体影", shadowIntensity);
    }

    if (isCapMan) {
        al::initJointControllerKeeper(this, 2);
        rs::initCapWorldNpcTail(this);
    } else {
        al::initJointControllerKeeper(this, 1);
    }

    mJointLookAtController = rs::tryCreateAndAppendNpcJointLookAtController(this, mTalkNpcParam);

    mStateScare = new NpcEventStateScare(this, &sScareParam);

    if (isCityMan)
        mStateReaction = NpcStateReaction::createForHuman(this, &sReactionParam);
    else
        mStateReaction = NpcStateReaction::create(this, &sReactionParam);

    al::initNerveState(this, mStateScare, &Scare, u8"イベント中の怖がり");
    al::initNerveState(this, mStateReaction, &Reaction, u8"リアクション");

    mMessageSystem = info.layoutInitInfo->getMessageSystem();

    mEventFlowExecutor = rs::initEventFlow(this, info, nullptr, nullptr);
    rs::initEventCharacterName(mEventFlowExecutor, info, "HelpNpc");
    rs::initEventParam(mEventFlowExecutor, mTalkNpcParam, nullptr);
    al::initEventReceiver(mEventFlowExecutor, this);
    rs::initEventCameraTalk(mEventFlowExecutor, info, "Default", -1.0f);

    auto* executor = mEventFlowExecutor;
    auto* turn = new EventActorMovementTurn(this, mTalkNpcParam, mActionAnimInfo);
    rs::initEventMovement(executor, turn, info);

    rs::startEventFlow(mEventFlowExecutor, "Wait");

    makeActorAlive();
}

bool HelpNpc::receiveEvent(const al::EventFlowEventData* event) {
    if (!al::isEqualString(event->getEventName(), "OpenHelpWindow"))
        return false;
    rs::requestShowHtmlViewer(this);
    return true;
}

bool HelpNpc::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other, al::HitSensor* self) {
    if (rs::isMsgPlayerDisregardHomingAttack(msg) || rs::isMsgPlayerDisregardTargetMarker(msg) ||
        (al::isMsgPlayerDisregard(msg) && mIsCapTarget && !al::isSensorName(self, "Head")))
        return true;

    if (mStateReaction->receiveMsg(msg, other, self)) {
        if (!al::isNerve(this, &Reaction))
            al::setNerve(this, &Reaction);
        return true;
    }

    return mStateReaction->receiveMsgNoReaction(msg, other, self);
}

void HelpNpc::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isSensorEye(self)) {
        if (mEventFlowExecutor)
            rs::sendMsgEventFlowScareCheck(other, self, mEventFlowExecutor);
        return;
    }
    rs::attackSensorNpcCommon(self, other);
}

void HelpNpc::control() {
    if (mJointLookAtController)
        rs::updateNpcJointLookAtController(mJointLookAtController);
}

const al::MessageSystem* HelpNpc::getMessageSystem() const {
    return mMessageSystem;
}

void HelpNpc::exeWait() {
    if (al::isFirstStep(this))
        rs::restartEventFlow(mEventFlowExecutor);

    if (mStateScare->tryStart(mEventFlowExecutor)) {
        rs::stopEventFlow(mEventFlowExecutor);
        al::setNerve(this, &Scare);
    } else if (rs::updateEventFlow(mEventFlowExecutor)) {
        rs::startEventFlow(mEventFlowExecutor, "Wait");
    }
}

void HelpNpc::exeScare() {
    rs::updateEventFlow(mEventFlowExecutor);
    al::updateNerveStateAndNextNerve(this, &Wait);
}

void HelpNpc::exeReaction() {
    al::updateNerveStateAndNextNerve(this, &Wait);
}
