#include "Npc/NpcEventCtrlInfo.h"

#include <cmath>
#include <cstring>

#include "Library/Camera/CameraUtil.h"
#include "Library/Camera/IUseCamera.h"
#include "Library/Event/EventFlowChoiceInfo.h"
#include "Library/Event/EventFlowDataHolder.h"
#include "Library/Event/EventFlowExecutorHolder.h"
#include "Library/Event/EventFlowNode.h"
#include "Library/Event/SceneEventFlowMsg.h"
#include "Library/Layout/BalloonOrderGroupHolder.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"

#include "Npc/NpcEventSceneInfo.h"
#include "Util/PlayerUtil.h"

class CostumePatternChecker {
public:
    CostumePatternChecker();

private:
    void* _00;
    s32 _08;
};

NpcEventCtrlInfo::NpcEventCtrlInfo(const NpcEventSceneInfo& sceneInfo,
                                   const NpcEventSceneConstData& sceneConstData,
                                   EventFlowSceneExecuteCtrl* execCtrl)
    : mSceneInfo(&sceneInfo), mSceneConstData(&sceneConstData), mExecCtrl(execCtrl) {
    mBalloonFilter = new BalloonFilterInfo();
    mCostumePatternChecker = new CostumePatternChecker();
    mSceneEventFlowMsg = new al::SceneEventFlowMsg();
    mExecutorHolder = new al::EventFlowExecutorHolder(1024);
    mBalloonGroupHolder = new al::BalloonOrderGroupHolder();
}

NpcEventCtrlInfo::~NpcEventCtrlInfo() = default;

bool NpcEventCtrlInfo::isCloseTalk() const {
    return mTalkInfo.mMessage == nullptr && mSceneInfo->_12;
}

// NON_MATCHING: compiler inlines struct copy as field-by-field loads/stores
// but our compiler generates a memcpy call for the 0x4A-byte copy
void NpcEventCtrlInfo::popBalloonInfo(NpcEventBalloonInfo* info) {
    *info = mBalloonInfo;
    mBalloonInfo._20 = 0;
    mBalloonInfo._24 = 0;
    mBalloonInfo._28 = 0;
    mBalloonInfo.mActor = nullptr;
    mBalloonInfo.mMessage = nullptr;
    mBalloonInfo.mType = -1;
    mBalloonInfo.mIconName = nullptr;
    mBalloonInfo._2c = 1.0f;
    mBalloonInfo._34 = -1;
    mBalloonInfo._38 = false;
    mBalloonInfo.mTagDataHolder = nullptr;
}

void NpcEventCtrlInfo::popTalkInfo(NpcEventTalkInfo* dest) {
    std::memcpy(dest, &mTalkInfo, sizeof(NpcEventTalkInfo));
    mTalkInfo.mTagDataHolder = nullptr;
    mTalkInfo.mActor = nullptr;
    mTalkInfo.mMessage = nullptr;
    mTalkInfo._08 = nullptr;
    mTalkInfo._20 = 0;
    mTalkInfo._24 = -1;
}

// NON_MATCHING: complex camera visibility and priority comparison logic; expects
// matching register allocation for camera direction dot products and distance comparisons
void NpcEventCtrlInfo::requestShowBalloonMessage(const NpcEventBalloonInfo& info) {
    if (mBalloonFilter->actor && mBalloonFilter->type != info._34)
        return;

    const al::LiveActor* actor = info.mActor;
    const al::IUseCamera* camera =
        actor ? static_cast<const al::IUseCamera*>(actor) : nullptr;

    const sead::Vector3f& camAt = al::getCameraAt(camera, 0);
    const sead::Vector3f& camPos = al::getCameraPos(camera, 0);

    f32 dirX = camAt.x - camPos.x;
    f32 dirY = camAt.y - camPos.y;
    f32 dirZ = camAt.z - camPos.z;

    const sead::Vector3f& actorTrans = al::getTrans(actor);
    const sead::Vector3f& camPos2 = al::getCameraPos(camera, 0);
    f32 dot1 = dirX * (actorTrans.x - camPos2.x) + dirY * (actorTrans.y - camPos2.y) +
               dirZ * (actorTrans.z - camPos2.z);
    if (dot1 < 0.0f)
        return;

    const sead::Vector3f& trans2 = al::getTrans(actor);
    f32 offsetX = trans2.x + *(reinterpret_cast<const f32*>(&info._20));
    f32 offsetY = trans2.y + *(reinterpret_cast<const f32*>(&info._24));
    f32 offsetZ = trans2.z + *(reinterpret_cast<const f32*>(&info._28));
    const sead::Vector3f& camPos3 = al::getCameraPos(camera, 0);
    f32 dot2 = dirX * (offsetX - camPos3.x) + dirY * (offsetY - camPos3.y) +
               dirZ * (offsetZ - camPos3.z);
    if (dot2 < 0.0f)
        return;

    const al::LiveActor* currentActor = mBalloonInfo.mActor;
    if (currentActor) {
        if (info._34 < mBalloonInfo._34)
            return;

        if (info._34 == mBalloonInfo._34) {
            if (!al::isNormalize(mSceneInfo->_28, 0.001f))
                goto accept;

            sead::Vector3f diff1;
            {
                const sead::Vector3f& curTrans = al::getTrans(currentActor);
                const sead::Vector3f& playerPos = rs::getPlayerPos(actor);
                diff1.x = curTrans.x - playerPos.x;
                diff1.y = curTrans.y - playerPos.y;
                diff1.z = curTrans.z - playerPos.z;
            }

            sead::Vector3f diff2;
            {
                const sead::Vector3f& newTrans = al::getTrans(actor);
                const sead::Vector3f& playerPos2 = rs::getPlayerPos(actor);
                diff2.x = newTrans.x - playerPos2.x;
                diff2.y = newTrans.y - playerPos2.y;
                diff2.z = newTrans.z - playerPos2.z;
            }

            if (al::isNearZero(diff1, 0.001f))
                goto accept;
            if (al::isNearZero(diff2, 0.001f))
                goto accept;

            const sead::Vector3f& viewDir = mSceneInfo->_28;
            f32 d1 = viewDir.x * diff1.x + viewDir.y * diff1.y + viewDir.z * diff1.z;
            f32 d2 = viewDir.x * diff2.x + viewDir.y * diff2.y + viewDir.z * diff2.z;

            if (d1 > 0.0f && d2 < 0.0f)
                return;

            if (!al::isSameSign(d1, d2))
                goto accept;

            {
                const sead::Vector3f& pp1 = rs::getPlayerPos(currentActor);
                const sead::Vector3f& t1 = al::getTrans(currentActor);
                f32 dist1 = std::sqrt((pp1.x - t1.x) * (pp1.x - t1.x) +
                                      (pp1.y - t1.y) * (pp1.y - t1.y) +
                                      (pp1.z - t1.z) * (pp1.z - t1.z));

                const sead::Vector3f& pp2 = rs::getPlayerPos(actor);
                const sead::Vector3f& t2 = al::getTrans(actor);
                f32 dist2 = std::sqrt((pp2.x - t2.x) * (pp2.x - t2.x) +
                                      (pp2.y - t2.y) * (pp2.y - t2.y) +
                                      (pp2.z - t2.z) * (pp2.z - t2.z));

                if (dist1 < dist2)
                    return;
            }
        }
    }

accept:
    mBalloonInfo = info;
}

void NpcEventCtrlInfo::requestShowTalkMessage(const al::EventFlowNode* node,
                                              const NpcEventTalkInfo& src) {
    std::memcpy(&mTalkInfo, &src, sizeof(NpcEventTalkInfo));
    const char* charName = node->getEventFlowDataHolder()->tryGetCharacterName();
    if (charName)
        mTalkInfo._08 = (void*)node->getEventFlowDataHolder()->tryGetCharacterName();
}

void NpcEventCtrlInfo::requestCloseTalkMessage(const al::LiveActor* actor) {
    reinterpret_cast<u8*>(&mTalkInfo)[0x23] = 1;
}

void NpcEventCtrlInfo::requestCloseWipeFadeBlack(al::EventFlowNode* node,
                                                 s32 frames) {
    mIsCloseWipeRequested = true;
    mCloseWipeFrames = frames;
}

void NpcEventCtrlInfo::requestOpenWipeFadeBlack(al::EventFlowNode* node,
                                                s32 frames) {
    mIsOpenWipeRequested = true;
    mOpenWipeFrames = frames;
}

void NpcEventCtrlInfo::setBalloonFilterOnlyMiniGame(
    const al::LiveActor* actor) {
    BalloonFilterInfo* filter = mBalloonFilter;
    if (!filter->actor) {
        filter->actor = actor;
        filter->type = 1;
    }
}

void NpcEventCtrlInfo::resetBalloonFilter(const al::LiveActor* actor) {
    BalloonFilterInfo* filter = mBalloonFilter;
    if (filter->actor && filter->actor == actor) {
        filter->actor = nullptr;
        filter->type = -1;
    }
}

void NpcEventCtrlInfo::startChoice(const al::EventFlowNode* node,
                                   al::EventFlowChoiceInfo* choiceInfo) {
    static constexpr char16 sEmptyStr[1] = {};
    const char16* talkMsg = choiceInfo->mTalkMessage;
    talkMsg = talkMsg == nullptr ? sEmptyStr : talkMsg;
    al::LiveActor* actor = node->getActor();
    auto* dataHolder = node->getEventFlowDataHolder();
    auto* tagData = *reinterpret_cast<const al::MessageTagDataHolder* const*>(
        reinterpret_cast<const u8*>(dataHolder) + 0x80);
    mTalkInfo.mMessage = talkMsg;
    mTalkInfo._08 = nullptr;
    mTalkInfo.mTagDataHolder = tagData;
    mTalkInfo.mActor = actor;
    mTalkInfo._20 = 0;
    mTalkInfo._24 = -1;
    const char* charName = node->getEventFlowDataHolder()->tryGetCharacterName();
    if (charName)
        mTalkInfo._08 = (void*)node->getEventFlowDataHolder()->tryGetCharacterName();
    mChoiceInfo = choiceInfo;
    choiceInfo->mSelectedChoice = -1;
}

s32 NpcEventCtrlInfo::getChoiceMessageNum() const {
    return mChoiceInfo->mMessageNum;
}

const char16* NpcEventCtrlInfo::getChoiceMessage(s32 index) const {
    return mChoiceInfo->mMessages[index];
}

s32 NpcEventCtrlInfo::getChoiceCancelIndex() const {
    return mChoiceInfo->mCancelIndex;
}

const char16* NpcEventCtrlInfo::tryGetChoiceTalkMessage() const {
    return mChoiceInfo->mTalkMessage;
}

void NpcEventCtrlInfo::endChoice(s32 selectedChoice) {
    mChoiceInfo->mSelectedChoice = selectedChoice;
    mChoiceInfo = nullptr;
}

bool NpcEventCtrlInfo::isEnableCancelChoice() const {
    return mChoiceInfo->mCancelIndex >= 0;
}

const char* NpcEventCtrlInfo::getSceneObjName() const {
    return u8"NPCイベント操作情報";
}
