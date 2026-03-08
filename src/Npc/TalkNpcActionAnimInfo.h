#pragma once

#include <basis/seadTypes.h>

namespace sead {
template <typename T>
class BufferedSafeStringBase;
}

namespace al {
struct ActorInitInfo;
class LiveActor;
}  // namespace al

struct NpcActionAnimParam;
class NpcStateReactionParam;
class TalkNpcParam;

class TalkNpcActionAnimInfo {
public:
    TalkNpcActionAnimInfo();

    virtual const char* convertActionName(sead::BufferedSafeStringBase<char>* buffer,
                                          const char* actionName) const;

    static const char* getArgWaitActionName(const al::ActorInitInfo& initInfo);
    void initWaitActionNameFromPlacementInfo(const al::LiveActor* actor,
                                             const al::ActorInitInfo& initInfo,
                                             bool isCheckHackWait);
    void initWaitActionNameDirect(const al::LiveActor* actor, const char* actionName,
                                  bool isCheckHackWait);
    void init(const al::LiveActor* actor, const al::ActorInitInfo& initInfo,
              const TalkNpcParam* param, const char* suffix);
    const char* getWaitActionName() const;
    const char* tryGetActorParamSuffix() const;
    bool tryApplyVisAnim(al::LiveActor* actor) const;
    void changeWaitActionName(const char* actionName, const TalkNpcParam* param);
    void changeHackWaitActionName(const char* actionName, const TalkNpcParam* param);
    void onHackWaitActionName(const TalkNpcParam* param);
    void offHackWaitActionName(const TalkNpcParam* param);
    bool changeWaitActionNameBySwitch(const char* actionName, const TalkNpcParam* param);
    bool resetWaitActionNameBySwitch(const TalkNpcParam* param);
    bool isSelectedInitWaitAction() const;
    const char* getAnyRandomActionName() const;

    const al::LiveActor* _8 = nullptr;
    const NpcActionAnimParam* _10 = nullptr;
    NpcStateReactionParam* _18 = nullptr;
    const char* mWaitActionName = nullptr;
    const char* _28 = "Surprise";
    bool _30 = false;
    u8 _pad31[7];
    const char* _38 = nullptr;
    const char* _40 = nullptr;
    const char* _48 = nullptr;
    s32 _50 = 0;
    u8 _pad54[4];
    const char** _58 = nullptr;
    bool _60 = true;
    bool _61 = true;
    u8 _pad62[6];
};

static_assert(sizeof(TalkNpcActionAnimInfo) == 0x68);
