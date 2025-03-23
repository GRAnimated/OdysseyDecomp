#pragma once

#include "Library/LiveActor/LiveActor.h"

class TalkNpcCap : public al::LiveActor {
public:
    static TalkNpcCap* tryCreate(const al::LiveActor*, const al::ActorInitInfo&);
    void initAttach(const al::LiveActor*);
    static TalkNpcCap* createForAchievementNpc(const al::LiveActor*, const al::ActorInitInfo&);
    static TalkNpcCap* createForHintNpc(const al::LiveActor*, const al::ActorInitInfo&);
    static TalkNpcCap* createForShibaken(const al::LiveActor*, const al::ActorInitInfo&);
    static TalkNpcCap* createForShoppingNpc(const al::LiveActor*, const al::ActorInitInfo&);
    static TalkNpcCap* createForShoppingNpcChromakey(const al::LiveActor*,
                                                     const al::ActorInitInfo&);
    static TalkNpcCap* createForVolleyballNpc(const al::LiveActor*, const al::ActorInitInfo&);
    void makeActorAlive() override;
    void control() override;
    void init(const al::ActorInitInfo&) override;

private:
    void* filler[39];
};
