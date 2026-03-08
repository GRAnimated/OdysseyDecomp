#include "Npc/NpcEventBalloonInfo.h"

#include <prim/seadSafeString.h>

#include "Library/Base/StringUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Message/MessageHolder.h"

#include "Layout/ProjectReplaceTagProcessor.h"

NpcEventBalloonInfo::NpcEventBalloonInfo() = default;

// NON_MATCHING: store coalescing differs (STR+STUR vs STP)
void NpcEventBalloonInfo::reset() {
    *this = NpcEventBalloonInfo();
}

// NON_MATCHING: store coalescing differs (STP XZR,XZR vs STP XZR+STR XZR)
void NpcEventBalloonInfo::setupForMessageBalloon(const al::LiveActor* actor, const char16* message,
                                                 const al::MessageTagDataHolder* tagDataHolder) {
    _28 = 0;
    _2c = 1.0f;
    mIconName = nullptr;
    _20 = 0;
    _24 = 0;
    _34 = -1;
    _38 = false;
    mTagDataHolder = nullptr;
    mType = 0;
    mActor = actor;
    mMessage = message;
    if (tagDataHolder)
        mTagDataHolder = tagDataHolder;
}

void NpcEventBalloonInfo::setupForEmotionIconBalloon(const al::LiveActor* actor,
                                                     const char* iconName) {
    _28 = 0;
    _2c = 1.0f;
    mIconName = iconName;
    _20 = 0;
    _24 = 0;
    _34 = -1;
    _38 = false;
    mTagDataHolder = nullptr;
    mType = 1;
    mActor = actor;
    mMessage = nullptr;
}

void NpcEventBalloonInfo::setupForTalkIconBalloon(const al::LiveActor* actor, const char* iconName,
                                                  bool a3) {
    _28 = 0;
    _2c = 1.0f;
    mIconName = iconName;
    _20 = 0;
    _24 = 0;
    _34 = -1;
    mType = 2;
    _38 = false;
    mTagDataHolder = nullptr;
    _48 = a3;
    mActor = actor;
    mMessage = nullptr;
}

// NON_MATCHING: adjacent fields coalesced into LDUR+STR QWORD vs separate LDR+STR pairs
void NpcEventBalloonInfo::setCommonParam(const NpcEventBalloonRequestInfo& requestInfo) {
    _28 = requestInfo._0c;
    _20 = requestInfo._04;
    _24 = requestInfo._08;
    _34 = requestInfo._00;
    _2c = requestInfo._10;
    _30 = requestInfo._14;
    _38 = requestInfo._18;
}

// NON_MATCHING: WStringTmp ctor devirtualizes assureTerminationImpl_ (BL vs BLR)
void NpcEventBalloonInfo::makeTextW(sead::BufferedSafeStringBase<char16>* outBuf,
                                    const al::IUseMessageSystem* msgSystem) const {
    ProjectReplaceTagProcessor tagProcessor(mActor);
    al::WStringTmp<1024> tempStr;
    tagProcessor.replace(tempStr.getBuffer(), msgSystem, mMessage);

    if (mTagDataHolder)
        al::replaceMessageTagData(outBuf, msgSystem, mTagDataHolder, tempStr.getBuffer());
    else
        al::copyMessageWithTag(outBuf->getBuffer(), outBuf->getBufferSize(), tempStr.getBuffer());
}

bool NpcEventBalloonInfo::isTypeMessage() const {
    return mType == 0;
}

bool NpcEventBalloonInfo::isTypeEmotionIcon() const {
    return mType == 1;
}

bool NpcEventBalloonInfo::isTypeTalkIcon() const {
    return mType == 2;
}

NpcEventTalkInfo::NpcEventTalkInfo() = default;

NpcEventTalkInfo::NpcEventTalkInfo(const al::LiveActor* actor, const char16* message,
                                   const al::MessageTagDataHolder* tagDataHolder)
    : mMessage(message), mTagDataHolder(tagDataHolder), mActor(actor) {}

void NpcEventTalkInfo::reset() {
    NpcEventTalkInfo defaultInfo;
    mTagDataHolder = defaultInfo.mTagDataHolder;
    mActor = defaultInfo.mActor;
    mMessage = defaultInfo.mMessage;
    _08 = defaultInfo._08;
    _20 = defaultInfo._20;
    _24 = defaultInfo._24;
}
