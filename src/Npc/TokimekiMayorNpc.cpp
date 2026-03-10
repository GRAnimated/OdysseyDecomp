#include "Npc/TokimekiMayorNpc.h"

#include "Library/Base/StringUtil.h"
#include "Library/Event/EventFlowFunction.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/Joint/JointControllerKeeper.h"
#include "Library/Joint/JointSpringControllerHolder.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorInitInfo.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/LiveActorFunction.h"
#include "Library/LiveActor/LiveActorGroup.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementInfo.h"
#include "Library/Stage/StageSwitchUtil.h"

#include "Npc/MayorItem.h"
#include "Npc/NpcStateReaction.h"
#include "Npc/NpcStateReactionParam.h"
#include "Npc/NpcStateWait.h"
#include "Npc/NpcStateWaitParam.h"
#include "Npc/RandomWaitActionUpdater.h"
#include "Npc/TalkNpcParam.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"
#include "System/GameDataUtil.h"
#include "System/SaveObjInfo.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"
#include "Util/PlayerDemoUtil.h"

namespace {
NERVE_IMPL(TokimekiMayorNpc, Wait);
NERVE_IMPL(TokimekiMayorNpc, Reaction);
NERVES_MAKE_NOSTRUCT(TokimekiMayorNpc, Wait, Reaction);

static NpcStateWaitParam sWaitParam("WaitSmile", nullptr, "Turn", "Reaction", nullptr, "Reaction",
                                    false, nullptr, false);
}  // namespace

TokimekiMayorNpc::TokimekiMayorNpc(const char* name) : al::LiveActor(name) {}

// NON_MATCHING: regswap (x22/x23 assignment), x27 used for mSaveObjInfoPresent address cache,
// stack layout differs (PlacementInfo at sp vs sp+0x10), loop cmp #0 vs #1
void TokimekiMayorNpc::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, "CityMayor", "Tokimeki");
    mEventFlowExecutor = rs::initEventFlow(this, info, nullptr, nullptr);
    al::initEventReceiver(mEventFlowExecutor, this);
    al::initNerve(this, &Wait, 4);

    mSaveObjInfo = rs::createSaveObjInfoNoWriteSaveDataInSameWorld(info);
    mFavorabilityRating = rs::getTokimekiMayorNpcFavorabilityRating(this);

    mTalkNpcParam = rs::initTalkNpcParam(this, nullptr);
    rs::initEventParam(mEventFlowExecutor, mTalkNpcParam, nullptr);
    rs::initEventCameraObject(mEventFlowExecutor, info, u8"会話カメラ");
    rs::initEventCameraObject(mEventFlowExecutor, info, u8"ときめき市長ムーンゲット");
    rs::initEventCameraObject(mEventFlowExecutor, info, u8"ときめき市長会話終了");
    rs::initEventCharacterName(mEventFlowExecutor, info, "Mayor");

    mNpcStateWait = new NpcStateWait(this, info, &sWaitParam, nullptr, nullptr);
    al::initNerveState(this, mNpcStateWait, &Wait, u8"待機");

    mNpcStateReactionParam = new NpcStateReactionParam();
    mNpcStateReaction = NpcStateReaction::create(this, nullptr);
    mNpcStateReaction->mParam = mNpcStateReactionParam;
    mNpcStateReaction->_29 = true;
    al::initNerveState(this, mNpcStateReaction, &Reaction, u8"リアクション");

    al::initJointControllerKeeper(this, 8);
    mNpcJointLookAtController = rs::tryCreateAndAppendNpcJointLookAtController(this, mTalkNpcParam);
    mJointSpringControllerHolder = new al::JointSpringControllerHolder();
    mJointSpringControllerHolder->init(this, "InitJointSpringCtrl");

    mQuestionBuffer.tryAllocBuffer(10, nullptr);

    s32 childCount = al::calcLinkChildNum(info, "MayorItem");
    mMayorItemGroup = new al::DeriveActorGroup<MayorItem>(u8"市長プレゼント", childCount);

    for (s32 i = 0; i < childCount; i++) {
        al::PlacementInfo placementInfo;
        al::getLinksInfoByIndex(&placementInfo, *info.placementInfo, "MayorItem", i);
        const char* displayName = nullptr;
        al::getDisplayName(&displayName, placementInfo);
        auto* item = new MayorItem(displayName);
        al::initLinksActor(item, info, "MayorItem", i);
        mMayorItemGroup->registerActor(item);
        if (al::isModelName(item, "PresentCorrect"))
            mSaveObjInfoPresent =
                rs::createSaveObjInfoNoWriteSaveDataInSameWorld(info, placementInfo);
    }

    GameDataHolderAccessor accessor(this);
    s32 shineIdx = GameDataFunction::tryFindLinkedShineIndex(this, info);
    if (GameDataFunction::isGotShine(accessor, shineIdx))
        mSaveObjInfoPresent->on();

    al::tryGetLinksQT(&mPlayerTalkQuat, &mPlayerTalkTrans, info, "PlayerTalkPos");

    mRandomWaitActionUpdater =
        new RandomWaitActionUpdater(this, info, mTalkNpcParam, "WaitSmile", "ByeBye");

    al::startAction(this, "WaitSmile");
    al::startVisAnim(this, "HatOn");
    rs::startEventFlow(mEventFlowExecutor, "Default");

    NpcStateReactionParam* reactionParam = mNpcStateReactionParam;
    if (mSaveObjInfoPresent->isOn()) {
        for (s32 i = 0; i < mMayorItemGroup->getActorCount(); i++)
            mMayorItemGroup->getActor(i)->kill();
        _160 = true;
        _161 = true;
        al::getSubActor(this, u8"プレゼント")->appear();
        reactionParam->mReactionAnim = "BagReaction";
        reactionParam->mReactionEndAnim = "BagReactionCap";
        al::onStageSwitch(this, "SwitchSuccessOn");
        rs::startEventFlow(mEventFlowExecutor, "Joy");
    } else if (mFavorabilityRating >= 4) {
        for (s32 i = 0; i < mMayorItemGroup->getActorCount(); i++)
            static_cast<MayorItem*>(mMayorItemGroup->getActor(i))->acceptPresent();
    }

    makeActorAlive();
}

void TokimekiMayorNpc::control() {
    if (!mSaveObjInfoPresent->isOn() && !al::isNerve(this, &Reaction))
        mRandomWaitActionUpdater->update();
    rs::updateNpcJointLookAtController(mNpcJointLookAtController);
    if (!_160)
        rs::tryUpdateNpcEyeLineAnim(this, mTalkNpcParam);
    rs::syncActionCityMayorFace(this);
    rs::syncMtsAnimCityMayorFace(this);
    if (mTalkNpcParam->isValidFacialAnim())
        mTalkNpcParam->updateFacialAnim(this);
    rs::trySwitchDepthToSelfShadow(this);
}

bool TokimekiMayorNpc::receiveMsg(const al::SensorMsg* msg, al::HitSensor* self,
                                  al::HitSensor* other) {
    if (rs::isMsgPlayerDisregardHomingAttack(msg) ||
        rs::tryReceiveMsgPlayerDisregard(msg, other, mTalkNpcParam))
        return true;

    bool result;
    if (rs::isInvalidTrampleSensor(other, mTalkNpcParam))
        result = mNpcStateReaction->receiveMsgWithoutTrample(msg, self, other);
    else
        result = mNpcStateReaction->receiveMsg(msg, self, other);

    if (result) {
        if (!al::isNerve(this, &Reaction))
            al::setNerve(this, &Reaction);
        return true;
    }

    return mNpcStateReaction->receiveMsgNoReaction(msg, self, other);
}

void TokimekiMayorNpc::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    rs::attackSensorNpcCommon(self, other);
}

// NON_MATCHING: stack frame 0x170 vs 0x1C0 (StringTmp locals not overlapped in target),
// ring buffer inlines generate different regalloc, many regswaps
bool TokimekiMayorNpc::receiveEvent(const al::EventFlowEventData* event) {
    if (al::isEventName(event, "Wait"))
        return true;

    if (al::isEventName(event, "TalkStart")) {
        _160 = true;
        _161 = false;
        rs::invalidateNpcJointLookAtController(mNpcJointLookAtController);
        rs::resetNpcEyeLineAnim(this);
        rs::replaceDemoPlayer(this, mPlayerTalkTrans, mPlayerTalkQuat);
        return true;
    }

    if (al::isEventName(event, "TalkEnd")) {
        _160 = false;
        rs::validateNpcJointLookAtController(mNpcJointLookAtController);
        mRandomWaitActionUpdater->setDisableBalloonAction();
        return true;
    }

    if (al::isEventName(event, "JumpRandomEntry")) {
        const char* eventName;

        if (mFavorabilityRating >= 4) {
            eventName = "PresentNoItem";
            for (s32 i = 0; i < mMayorItemGroup->getActorCount(); i++) {
                auto* item = static_cast<MayorItem*>(mMayorItemGroup->getActor(i));
                if (item->isHold()) {
                    const char* modelName = al::getModelName(item);
                    if (al::isEqualString(modelName, "PresentWrongA")) {
                        eventName = "PresentWrongA";
                        break;
                    }
                    if (al::isEqualString(modelName, "PresentWrongB")) {
                        eventName = "PresentWrongB";
                        break;
                    }
                    if (al::isEqualString(modelName, "PresentCorrect")) {
                        eventName = "PresentCorrect";
                        break;
                    }
                }
            }
        } else if (mFavorabilityRating == 0 && !_161) {
            _161 = true;
            if (mSaveObjInfo->isOn()) {
                eventName = "Retry";
            } else {
                mSaveObjInfo->on();
                eventName = "Introduction";
            }
        } else {
            if (mQuestionBuffer.size() == 0) {
                for (s32 i = 0; i < mQuestionBuffer.capacity(); i++)
                    mQuestionBuffer.pushBack(i);

                for (s32 k = mQuestionBuffer.size() - 1; k >= 1; k--) {
                    s32 r = al::getRandom(k + 1);
                    s32 a = mQuestionBuffer[k];
                    s32 b = mQuestionBuffer[r];
                    mQuestionBuffer[k] = b;
                    mQuestionBuffer[r] = a;
                }

                if (mQuestionBuffer.size() >= 2) {
                    s32 half = mQuestionBuffer.capacity() / 2;
                    for (s32 j = 0; j < half; j++) {
                        if (mQuestionBuffer[j] == mLastQuestionIndex) {
                            s32 swapIdx = j + half;
                            s32 tmp = mQuestionBuffer[swapIdx];
                            mQuestionBuffer[swapIdx] = mQuestionBuffer[j];
                            mQuestionBuffer[j] = tmp;
                            break;
                        }
                    }
                }
            }

            s32 questionIdx;
            if (mQuestionBuffer.size() > 0) {
                questionIdx = mQuestionBuffer.popFront();
                mLastQuestionIndex = questionIdx;
            } else {
                questionIdx = mLastQuestionIndex;
            }

            al::StringTmp<256> str("Q%02d", questionIdx + 1);
            eventName = str.cstr();
        }

        rs::startEventFlow(mEventFlowExecutor, eventName);
        return true;
    }

    if (al::isEventName(event, "FavorabilityUp")) {
        mFavorabilityRating++;
        rs::setTokimekiMayorNpcFavorabilityRating(this, mFavorabilityRating);
        al::StringTmp<64> str(u8"好感度アップ%d", mFavorabilityRating);
        al::startHitReaction(this, str.cstr());
        if (mFavorabilityRating >= 4)
            for (s32 i = 0; i < mMayorItemGroup->getActorCount(); i++)
                static_cast<MayorItem*>(mMayorItemGroup->getActor(i))->acceptPresent();
        return true;
    }

    if (al::isEventName(event, "FavorabilityDown")) {
        mFavorabilityRating = 0;
        rs::setTokimekiMayorNpcFavorabilityRating(this, 0);
        al::startHitReaction(this, u8"好感度ダウン");
    } else if (al::isEventName(event, "Success")) {
        mSaveObjInfoPresent->on();
        for (s32 i = 0; i < mMayorItemGroup->getActorCount(); i++)
            static_cast<MayorItem*>(mMayorItemGroup->getActor(i))->endPresent();
        al::onStageSwitch(this, "SwitchSuccessOn");
    } else if (al::isEventName(event, "ItemPresent")) {
        bool isCorrect = false;
        for (s32 i = 0; i < mMayorItemGroup->getActorCount(); i++) {
            auto* item = static_cast<MayorItem*>(mMayorItemGroup->getActor(i));
            if (item->isHold()) {
                item->collectPresent();
                isCorrect |= al::isEqualString(al::getModelName(item), "PresentCorrect");
            }
        }
        if (isCorrect) {
            al::getSubActor(this, u8"プレゼント")->appear();
            mNpcStateReactionParam->mReactionAnim = "BagReaction";
            mNpcStateReactionParam->mReactionEndAnim = "BagReactionCap";
        }
    } else {
        return false;
    }

    return true;
}

void TokimekiMayorNpc::exeWait() {
    rs::updateEventFlow(mEventFlowExecutor);
}

void TokimekiMayorNpc::exeReaction() {
    const char* eventFlowName = mSaveObjInfoPresent->isOn() ? "Joy" : "Default";
    if (rs::checkEnableStartEventAndCancelReaction(this, mTalkNpcParam)) {
        rs::startEventFlow(mEventFlowExecutor, eventFlowName);
        al::setNerve(this, &Wait);
    } else if (al::updateNerveStateAndNextNerve(this, &Wait)) {
        rs::startEventFlow(mEventFlowExecutor, eventFlowName);
    }
}
