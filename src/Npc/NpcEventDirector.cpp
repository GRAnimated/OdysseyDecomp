#include "Npc/NpcEventDirector.h"

#include <container/seadPtrArray.h>
#include <math/seadVector.h>

#include "Library/Area/AreaObj.h"
#include "Library/Area/AreaObjGroup.h"
#include "Library/Area/AreaObjUtil.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/Execute/ExecuteUtil.h"
#include "Library/Layout/BalloonOrderGroupHolder.h"
#include "Library/Layout/LayoutActionFunction.h"
#include "Library/Layout/LayoutActorGroup.h"
#include "Library/Layout/LayoutInitInfo.h"
#include "Library/LiveActor/ActorInitInfo.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Play/Layout/BalloonMessage.h"
#include "Library/Play/Layout/SimpleLayoutAppearWaitEnd.h"
#include "Library/Play/Layout/WipeSimple.h"
#include "Library/Player/PlayerUtil.h"
#include "Library/Scene/SceneObjUtil.h"
#include "Library/Se/SeFunction.h"

#include "Layout/TalkMessage.h"
#include "Npc/BirdPlayerGlideCtrl.h"
#include "Npc/EventDemoCtrl.h"
#include "Npc/EventFlowSceneExecuteCtrl.h"
#include "Npc/NpcEventBalloonInfo.h"
#include "Npc/NpcEventCtrlInfo.h"
#include "Npc/NpcEventSceneConstData.h"
#include "Npc/NpcEventSceneInfo.h"
#include "Util/DemoUtil.h"
#include "Util/PlayerDemoUtil.h"
#include "Util/PlayerUtil.h"

struct BalloonLayoutEntry {
    al::LayoutActor* current = nullptr;
    al::LayoutActorGroup* group = nullptr;
};

struct BalloonLayoutHolder {
    BalloonLayoutEntry* emote = nullptr;
    BalloonLayoutEntry* message = nullptr;
    s32 requestTimer = 0;
    s32 _14 = 0;
    sead::PtrArrayImpl* infos = nullptr;
};

struct TalkMessageHolder {
    s32 kind = 0;
    TalkMessage* talk = nullptr;
    TalkMessage* important = nullptr;
};

static NpcEventSceneConstData sSceneConstData;

namespace {
NERVE_IMPL(NpcEventDirector, Wait);
NERVE_IMPL(NpcEventDirector, Demo);
NERVE_END_IMPL(NpcEventDirector, DemoTalk);
NERVE_IMPL(NpcEventDirector, DemoTalkEnd);
NERVE_IMPL(NpcEventDirector, DemoWipeClose);
NERVE_IMPL(NpcEventDirector, DemoWipeOpen);
NERVE_IMPL(NpcEventDirector, DemoSelectChoiceStart);
NERVE_IMPL(NpcEventDirector, DemoSelectChoice);

NERVES_MAKE_NOSTRUCT(NpcEventDirector, Demo, DemoSelectChoice);
NERVES_MAKE_STRUCT(NpcEventDirector, Wait, DemoTalk, DemoTalkEnd, DemoWipeClose, DemoWipeOpen,
                   DemoSelectChoiceStart);
}  // namespace

static TalkMessage* getActiveTalkMessage(TalkMessageHolder* holder) {
    if (holder->kind == 2)
        return holder->important;
    if (holder->kind == 1)
        return holder->talk;
    return nullptr;
}

static bool tryStartDemoTalk(NpcEventDirector* director, NpcEventCtrlInfo* ctrlInfo,
                             NpcEventTalkInfo* talkInfo, TalkMessageHolder* holder) {
    // access mTalkInfo.mMessage at NpcEventCtrlInfo offset 0x90
    if (!*reinterpret_cast<const char16**>(reinterpret_cast<u8*>(ctrlInfo) + 0x90))
        return false;

    ctrlInfo->popTalkInfo(talkInfo);

    bool isImportant = reinterpret_cast<const u8*>(&talkInfo->_20)[1];
    holder->kind = isImportant ? 2 : 1;

    TalkMessage* msg = getActiveTalkMessage(holder);
    bool isNormal = reinterpret_cast<const u8*>(&talkInfo->_20)[0] == 0;
    msg->startForNpc(talkInfo->mActor, talkInfo->mMessage,
                     static_cast<const char16*>(talkInfo->_08), talkInfo->mTagDataHolder, isNormal);

    s32 pageCount = talkInfo->_24;
    if (pageCount >= 1) {
        TalkMessage* activeMsg = getActiveTalkMessage(holder);
        reinterpret_cast<s32*>(reinterpret_cast<u8*>(activeMsg) + 0x160)[0] = pageCount;
    }

    // access mChoiceInfo at NpcEventCtrlInfo offset 0xB8
    if (*reinterpret_cast<void**>(reinterpret_cast<u8*>(ctrlInfo) + 0xB8))
        al::setNerve(director, &NrvNpcEventDirector.DemoTalk);
    else
        al::setNerve(director, &NrvNpcEventDirector.DemoTalkEnd);

    return true;
}

NpcEventDirector::NpcEventDirector(const al::PlayerHolder* playerHolder,
                                   al::CameraDirector* cameraDirector,
                                   al::CollisionDirector* collisionDirector,
                                   const al::MessageSystem* messageSystem,
                                   EventFlowSceneExecuteCtrl* execCtrl)
    : al::NerveExecutor("Npc"), mNpcEventSceneInfo(nullptr), mNpcEventCtrlInfo(nullptr),
      _48(nullptr), _50(nullptr), mEventDemoCtrl(nullptr), mEventFlowSceneExecuteCtrl(execCtrl),
      mWipeSimple(nullptr), mWipeFrames(-1), mNpcEventBalloonInfo(nullptr),
      mNpcEventTalkInfo(nullptr), mPlayerHolder(playerHolder), mCameraDirector(cameraDirector),
      mCollisionDirector(collisionDirector), mSceneObjHolder(nullptr),
      mMessageSystem(messageSystem) {}

// NON_MATCHING: init creates anonymous subclasses of SimpleLayoutAppearWaitEnd (0x148) and
// BalloonMessage (0x9E8) with replaced vtables and extra fields — cannot replicate in C++
void NpcEventDirector::init(const al::ActorInitInfo& initInfo) {
    al::registerExecutorUser(this, initInfo.executeDirector, "Npc");
    initNerve(&NrvNpcEventDirector.Wait, 0);
    mSceneObjHolder = initInfo.actorSceneInfo.sceneObjHolder;

    mEventDemoCtrl = new EventDemoCtrl();
    mNpcEventBalloonInfo = new NpcEventBalloonInfo();
    auto* talkInfo = new NpcEventTalkInfo();
    mNpcEventTalkInfo = talkInfo;

    auto* sceneInfo = new NpcEventSceneInfo();
    sceneInfo->_08 = talkInfo;
    mNpcEventSceneInfo = sceneInfo;

    auto* ctrlInfo = new NpcEventCtrlInfo(*sceneInfo, sSceneConstData, mEventFlowSceneExecuteCtrl);
    mNpcEventCtrlInfo = ctrlInfo;

    auto* balloonHolder = new BalloonLayoutHolder();
    _48 = balloonHolder;

    const al::LayoutInitInfo& layoutInfo = al::getLayoutInitInfo(initInfo);

    auto* emoteEntry = new BalloonLayoutEntry();
    balloonHolder->emote = emoteEntry;
    auto* emoteGroup = new al::LayoutActorGroup("アイコンバルーングループ", 8);
    emoteEntry->group = emoteGroup;

    auto* messageEntry = new BalloonLayoutEntry();
    balloonHolder->message = messageEntry;
    auto* messageGroup = new al::LayoutActorGroup("メッセージバルーングループ", 8);
    messageEntry->group = messageGroup;

    auto* infos = new sead::PtrArrayImpl();
    infos->allocBuffer(16, nullptr, 8);
    balloonHolder->infos = infos;

    auto* talkHolder = new TalkMessageHolder();
    _50 = talkHolder;

    auto* talkMsg = new TalkMessage("イベント用会話ウインドウ");
    talkHolder->talk = talkMsg;
    talkMsg->initLayoutForEventTalk(layoutInfo);

    auto* importantMsg = new TalkMessage("イベント用会話ウインドウ");
    talkHolder->important = importantMsg;
    importantMsg->initLayoutForEventImportant(layoutInfo);

    auto* wipe = new al::WipeSimple("イベント用黒フェードワイプ", "FadeBlack",
                                    al::getLayoutInitInfo(initInfo), nullptr);
    mWipeSimple = wipe;
}

// NON_MATCHING: compiler generates different Vector3f copy pattern and computation ordering
void NpcEventDirector::execute() {
    auto* player = al::tryGetPlayerActor(mPlayerHolder, 0);
    NpcEventSceneInfo* sceneInfo = mNpcEventSceneInfo;

    if (player) {
        sceneInfo->_14 = true;
        if (!rs::tryCalcPlayerModelHeadJointPos(&sceneInfo->_1c, player)) {
            NpcEventSceneInfo* info = mNpcEventSceneInfo;
            const sead::Vector3f& headPos = rs::getPlayerHeadPos(player);
            info->_1c = headPos;
        }

        sead::Vector3f frontDir(0, 0, 0);
        rs::calcPlayerFrontDir(&frontDir, player);

        auto* info = mNpcEventSceneInfo;
        info->_28.x = info->_28.x * 0.8f + frontDir.x * 0.2f;
        info->_28.y = info->_28.y * 0.8f + frontDir.y * 0.2f;
        info->_28.z = info->_28.z * 0.8f + frontDir.z * 0.2f;

        if (!al::tryNormalizeOrZero(&mNpcEventSceneInfo->_28))
            mNpcEventSceneInfo->_28 = frontDir;

        mNpcEventSceneInfo->_34 = rs::isPlayerInWater(player) ? 1 : 0;

        auto* groupHolder = mNpcEventCtrlInfo->mBalloonGroupHolder;
        const sead::Vector3f& playerPos = rs::getPlayerPos(player);
        groupHolder->updateAll(playerPos);
    } else {
        sceneInfo->_14 = false;
        mNpcEventCtrlInfo->mBalloonGroupHolder->resetInsideTerritoryAll();
    }

    mEventFlowSceneExecuteCtrl->updateNerve();
    updateNerve();
    updateBalloon();

    alEventFlowFunction::clearSceneEventFlowMsg(mNpcEventCtrlInfo->mSceneEventFlowMsg);

    auto* balloonHolder = static_cast<BalloonLayoutHolder*>(_48);
    al::LayoutActor* currentEmote = balloonHolder->emote->current;
    void* layoutPtr;
    if (currentEmote)
        layoutPtr = reinterpret_cast<void**>(reinterpret_cast<u8*>(currentEmote) + 0x130)[0];
    else {
        al::LayoutActor* currentMsg = balloonHolder->message->current;
        if (currentMsg)
            layoutPtr = reinterpret_cast<void**>(reinterpret_cast<u8*>(currentMsg) + 0x130)[0];
        else
            layoutPtr = nullptr;
    }
    mNpcEventSceneInfo->_00 = layoutPtr;

    auto* currentBalloon = balloonHolder->message->current;
    bool voicePlaying = false;
    if (currentBalloon)
        voicePlaying = static_cast<al::BalloonMessage*>(currentBalloon)->isVoicePlayerPlaying();
    mNpcEventSceneInfo->_10 = voicePlaying;

    auto* talkHolder = static_cast<TalkMessageHolder*>(_50);
    TalkMessage* activeTalk = getActiveTalkMessage(talkHolder);
    bool isTalkWait = false;
    if (activeTalk)
        isTalkWait = activeTalk->isWait();
    mNpcEventSceneInfo->_11 = isTalkWait;

    activeTalk = getActiveTalkMessage(talkHolder);
    bool isTalkNotActive = true;
    if (activeTalk)
        isTalkNotActive = !activeTalk->isAlive();
    mNpcEventSceneInfo->_12 = isTalkNotActive;

    activeTalk = getActiveTalkMessage(talkHolder);
    bool isEndAnim = true;
    if (activeTalk) {
        auto* textPaneAnim = reinterpret_cast<void**>(reinterpret_cast<u8*>(activeTalk) + 0xF0)[0];
        if (textPaneAnim)
            isEndAnim = al::isEndTextPaneAnim(activeTalk, false);
        else
            isEndAnim = true;
    }
    mNpcEventSceneInfo->_13 = isEndAnim;

    al::WipeSimple* wipe = mWipeSimple;
    s32 wipeState;
    if (al::isNerve(this, &NrvNpcEventDirector.DemoWipeClose))
        wipeState = 0;
    else if (wipe->isAlive())
        wipeState = 1;
    else if (al::isNerve(this, &NrvNpcEventDirector.DemoWipeOpen))
        wipeState = 2;
    else
        wipeState = -1;
    mNpcEventSceneInfo->_18 = wipeState;

    auto* ctrlInfo = mNpcEventCtrlInfo;
    ctrlInfo->mIsCloseWipeRequested = false;
    ctrlInfo->mIsOpenWipeRequested = false;
    reinterpret_cast<u8*>(&ctrlInfo->mTalkInfo._20)[3] = 0;
}

// NON_MATCHING: extremely large function (3808 bytes) with complex balloon update logic
// involving raycasting, icon state machines, BalloonMessage text/voice/animation setup,
// and an unrolled 16-entry info array search
void NpcEventDirector::updateBalloon() {}

void NpcEventDirector::killAllBalloonForSnapShotMode() {
    mNpcEventBalloonInfo->reset();

    auto* balloonHolder = static_cast<BalloonLayoutHolder*>(_48);
    auto* emoteEntry = balloonHolder->emote;
    if (emoteEntry->current) {
        emoteEntry->current->kill();
        emoteEntry->current = nullptr;
    }
    auto* messageEntry = balloonHolder->message;
    if (messageEntry->current) {
        messageEntry->current->kill();
        messageEntry->current = nullptr;
    }
}

void NpcEventDirector::exeWait() {
    if (al::isFirstStep(this))
        static_cast<BalloonLayoutHolder*>(_48)->requestTimer = 15;

    if (!mEventDemoCtrl->isActiveDemo())
        return;

    auto* talkHolder = static_cast<TalkMessageHolder*>(_50);
    TalkMessage* talkMsg = talkHolder->talk;
    al::forceActivateSeKeeper(talkMsg ? static_cast<al::IUseAudioKeeper*>(talkMsg) : nullptr);
    talkMsg = talkHolder->talk;
    al::startSe(talkMsg ? static_cast<al::IUseAudioKeeper*>(talkMsg) : nullptr, "NpcTalkStartPg");

    if (al::isExistSceneObj(this, BirdPlayerGlideCtrl::sSceneObjId))
        al::getSceneObj<BirdPlayerGlideCtrl>(this)->addDemoActorAndFlyAway();

    if (!tryStartDemoTalk(this, mNpcEventCtrlInfo, mNpcEventTalkInfo,
                          static_cast<TalkMessageHolder*>(_50)))
        al::setNerve(this, &Demo);
}

// NON_MATCHING: getActiveTalkMessage comparison order and repeated-inline pattern
void NpcEventDirector::exeDemo() {
    if (tryStartDemoTalk(this, mNpcEventCtrlInfo, mNpcEventTalkInfo,
                         static_cast<TalkMessageHolder*>(_50)))
        return;

    auto* ctrlInfo = mNpcEventCtrlInfo;
    u64 wipeData = *reinterpret_cast<u64*>(&ctrlInfo->mIsCloseWipeRequested);
    ctrlInfo->mIsCloseWipeRequested = false;
    if (static_cast<u8>(wipeData)) {
        mWipeFrames = static_cast<s32>(wipeData >> 32);
        al::setNerve(this, &NrvNpcEventDirector.DemoWipeClose);
        return;
    }

    ctrlInfo = mNpcEventCtrlInfo;
    s32 openFrames = ctrlInfo->mOpenWipeFrames;
    bool openRequested = ctrlInfo->mIsOpenWipeRequested;
    ctrlInfo->mIsOpenWipeRequested = false;
    if (openRequested) {
        mWipeFrames = openFrames;
        al::setNerve(this, &NrvNpcEventDirector.DemoWipeOpen);
        return;
    }

    if (alEventFlowFunction::isReceiveCommandCloseTalkMessageLayout(
            mNpcEventCtrlInfo->mSceneEventFlowMsg)) {
        TalkMessage* msg = getActiveTalkMessage(static_cast<TalkMessageHolder*>(_50));
        if (msg && msg->isAlive()) {
            al::setNerve(this, &NrvNpcEventDirector.DemoSelectChoiceStart);
            return;
        }
    }

    if (reinterpret_cast<u8*>(&mNpcEventCtrlInfo->mTalkInfo._20)[3]) {
        TalkMessage* msg = getActiveTalkMessage(static_cast<TalkMessageHolder*>(_50));
        if (msg && msg->isAlive()) {
            al::setNerve(this, &NrvNpcEventDirector.DemoSelectChoiceStart);
            return;
        }
    }

    if (mEventDemoCtrl->isRequestEndDemo()) {
        TalkMessage* msg = getActiveTalkMessage(static_cast<TalkMessageHolder*>(_50));
        if (msg && msg->isAlive()) {
            al::setNerve(this, &NrvNpcEventDirector.DemoSelectChoiceStart);
            return;
        }
        mEventDemoCtrl->endDemo();
        al::setNerve(this, &NrvNpcEventDirector.Wait);
        return;
    }

    if (!mEventDemoCtrl->isActiveDemo())
        al::setNerve(this, &NrvNpcEventDirector.Wait);
}

void NpcEventDirector::exeDemoWipeClose() {
    if (al::isFirstStep(this))
        mWipeSimple->startClose(mWipeFrames);
    if (mWipeSimple->isCloseEnd())
        al::setNerve(this, &Demo);
}

void NpcEventDirector::exeDemoWipeOpen() {
    if (al::isFirstStep(this))
        mWipeSimple->startOpen(mWipeFrames);
    if (!mWipeSimple->isAlive())
        al::setNerve(this, &Demo);
}

// NON_MATCHING: getActiveTalkMessage comparison order differs (compiler checks kind==2 first)
void NpcEventDirector::exeDemoTalk() {
    bool forceClose = reinterpret_cast<u8*>(&mNpcEventCtrlInfo->mTalkInfo._20)[3] != 0;
    auto* talkHolder = static_cast<TalkMessageHolder*>(_50);

    if (forceClose) {
        TalkMessage* msg = getActiveTalkMessage(talkHolder);
        msg->kill();
        al::setNerve(this, &Demo);
        return;
    }

    TalkMessage* msg = getActiveTalkMessage(talkHolder);
    if (msg && msg->isWait())
        al::setNerve(this, &Demo);
}

void NpcEventDirector::endDemoTalk() {
    mNpcEventTalkInfo->reset();
}

// NON_MATCHING: getActiveTalkMessage comparison order and repeated-inline pattern
void NpcEventDirector::exeDemoTalkEnd() {
    if (al::isFirstStep(this)) {
        TalkMessage* msg = getActiveTalkMessage(static_cast<TalkMessageHolder*>(_50));
        msg->end();

        if (mEventDemoCtrl->isRequestEndDemo() && mEventDemoCtrl->isActiveDemoWithPlayer()) {
            const al::LiveActor* demoActor = mEventDemoCtrl->getDemoStartActor();
            rs::startActionDemoPlayer(demoActor, "TalkEnd");
        }
    }

    if (tryStartDemoTalk(this, mNpcEventCtrlInfo, mNpcEventTalkInfo,
                         static_cast<TalkMessageHolder*>(_50)))
        return;

    TalkMessage* msg = getActiveTalkMessage(static_cast<TalkMessageHolder*>(_50));
    if (msg && msg->isAlive())
        return;

    al::setNerve(this, &Demo);
}

// NON_MATCHING: getActiveTalkMessage comparison order differs
void NpcEventDirector::exeDemoSelectChoiceStart() {
    auto* talkHolder = static_cast<TalkMessageHolder*>(_50);
    TalkMessage* msg = getActiveTalkMessage(talkHolder);
    if (msg && msg->isWait())
        al::setNerve(this, &DemoSelectChoice);
}

void NpcEventDirector::exeDemoSelectChoice() {
    auto* talkHolder = static_cast<TalkMessageHolder*>(_50);

    if (al::isFirstStep(this)) {
        TalkMessage* msg = getActiveTalkMessage(talkHolder);
        msg->startSelectWithChoiceInfo(mNpcEventCtrlInfo->mChoiceInfo);
    }

    TalkMessage* msg = getActiveTalkMessage(talkHolder);
    if (msg->isSelectDecide()) {
        auto* ctrlInfo = mNpcEventCtrlInfo;
        TalkMessage* activeMsg = getActiveTalkMessage(talkHolder);
        s32 choice = activeMsg->getSelectedChoiceIndex();
        ctrlInfo->endChoice(choice);
        al::setNerve(this, &Demo);
    }
}
