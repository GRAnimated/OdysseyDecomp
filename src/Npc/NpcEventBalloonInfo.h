#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
class MessageTagDataHolder;
class IUseMessageSystem;
}  // namespace al

namespace sead {
template <typename T>
class BufferedSafeStringBase;
}

struct NpcEventBalloonRequestInfo {
    s32 _00;
    s32 _04;
    s32 _08;
    s32 _0c;
    f32 _10;
    s32 _14;
    bool _18;
};

class NpcEventBalloonInfo {
public:
    NpcEventBalloonInfo();
    void setupForMessageBalloon(const al::LiveActor* actor, const char16* message,
                                const al::MessageTagDataHolder* tagDataHolder);
    void reset();
    void setupForEmotionIconBalloon(const al::LiveActor* actor, const char* iconName);
    void setupForTalkIconBalloon(const al::LiveActor* actor, const char* iconName, bool a3);
    void setCommonParam(const NpcEventBalloonRequestInfo& requestInfo);
    void makeTextW(sead::BufferedSafeStringBase<char16>* outBuf,
                   const al::IUseMessageSystem* msgSystem) const;
    bool isTypeMessage() const;
    bool isTypeEmotionIcon() const;
    bool isTypeTalkIcon() const;

    const al::LiveActor* mActor = nullptr;
    const char16* mMessage = nullptr;
    s32 mType = -1;
    const char* mIconName = nullptr;
    s32 _20 = 0;
    s32 _24 = 0;
    s32 _28 = 0;
    f32 _2c = 1.0f;
    s32 _30 = 0;
    s32 _34 = -1;
    bool _38 = false;
    const al::MessageTagDataHolder* mTagDataHolder = nullptr;
    bool _48 = false;
    bool _49 = false;
};

static_assert(sizeof(NpcEventBalloonInfo) == 0x50);

class NpcEventTalkInfo {
public:
    NpcEventTalkInfo();
    NpcEventTalkInfo(const al::LiveActor* actor, const char16* message,
                     const al::MessageTagDataHolder* tagDataHolder);
    void reset();

    const char16* mMessage = nullptr;
    void* _08 = nullptr;
    const al::MessageTagDataHolder* mTagDataHolder = nullptr;
    const al::LiveActor* mActor = nullptr;
    s32 _20 = 0;
    s32 _24 = -1;
};

static_assert(sizeof(NpcEventTalkInfo) == 0x28);
