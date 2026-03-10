#include "Npc/TalkNpcSwitchActionPlayerHolder.h"

#include "Library/LiveActor/ActorInitInfo.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementInfo.h"

#include "Npc/TalkNpcSwitchActionPlayer.h"

TalkNpcSwitchActionPlayerHolder::TalkNpcSwitchActionPlayerHolder(
    TalkNpcSwitchActionPlayer** players, s32 count)
    : mPlayers(players), mCount(count) {}

TalkNpcSwitchActionPlayerHolder*
TalkNpcSwitchActionPlayerHolder::tryCreate(RandomActionUpdater* updater,
                                           const al::ActorInitInfo& initInfo) {
    s32 count = al::calcLinkChildNum(initInfo, "SwitchActionPlayer");
    if (count < 1)
        return nullptr;

    auto** players = new TalkNpcSwitchActionPlayer*[count];
    for (s32 i = 0; i < count; i++) {
        al::PlacementInfo placementInfo;
        al::getLinksInfoByIndex(&placementInfo, initInfo, "SwitchActionPlayer", i);
        players[i] =
            new TalkNpcSwitchActionPlayer(initInfo.stageSwitchDirector, updater, placementInfo);
    }

    return new TalkNpcSwitchActionPlayerHolder(players, count);
}
