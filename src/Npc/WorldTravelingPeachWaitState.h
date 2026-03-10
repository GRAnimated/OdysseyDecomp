#pragma once

#include <math/seadVector.h>

#include "Library/Event/EventFlowMovement.h"

class NpcStateTurnSeparate;
class TalkNpcActionAnimInfo;
class TalkNpcParam;
class Tiara;

class WorldTravelingPeachWaitState : public al::EventFlowMovement {
public:
    WorldTravelingPeachWaitState(al::LiveActor* actor, Tiara* tiara, const TalkNpcParam* param,
                                 const TalkNpcActionAnimInfo* animInfo);

    void appear() override;
    void kill() override;

    void startPeachWorldHomeCastleCapDemo();
    void endPeachWorldHomeCastleCapDemo();

    void exeWait();
    void exeTurnToTiara();
    void exeAdlibTalk();
    void exeTurnToBack();
    void exeFindTurn();
    void exeShakeHand();
    void exeTurnToPlayer();
    void exeWaitInitPeachWorldHomeCastleCapDemo();
    void exeShakeHandPeachWorldHomeCastleCapDemo();
    void exeWaitAfterPeachWorldHomeCastleCapDemo();

private:
    Tiara* mTiara;
    const TalkNpcParam* mTalkNpcParam;
    const TalkNpcActionAnimInfo* mAnimInfo;
    NpcStateTurnSeparate* mTurnSeparate;
    sead::Vector3f mFrontDir;
    bool _5c;
};

static_assert(sizeof(WorldTravelingPeachWaitState) == 0x60);
