#pragma once

#include <basis/seadTypes.h>

namespace al {
struct ActorInitInfo;
}  // namespace al

class RandomActionUpdater;
class TalkNpcSwitchActionPlayer;

class TalkNpcSwitchActionPlayerHolder {
public:
    TalkNpcSwitchActionPlayerHolder(TalkNpcSwitchActionPlayer** players, s32 count);

    static TalkNpcSwitchActionPlayerHolder* tryCreate(RandomActionUpdater* updater,
                                                      const al::ActorInitInfo& initInfo);

private:
    TalkNpcSwitchActionPlayer** mPlayers;
    s32 mCount;
};

static_assert(sizeof(TalkNpcSwitchActionPlayerHolder) == 0x10);
