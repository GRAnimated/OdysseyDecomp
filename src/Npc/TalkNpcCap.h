#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/LiveActor/LiveActor.h"

namespace al {
struct ActorInitInfo;
}

struct CapTypeData {
    const char* name;
    const char* objectDataPath;
    const char* japaneseName;
};

class TalkNpcCap : public al::LiveActor {
public:
    static TalkNpcCap* tryCreate(const al::LiveActor* parentActor,
                                 const al::ActorInitInfo& initInfo);
    void initAttach(const al::LiveActor* parentActor);
    static TalkNpcCap* createForAchievementNpc(const al::LiveActor* parentActor,
                                               const al::ActorInitInfo& initInfo);
    static TalkNpcCap* createForHintNpc(const al::LiveActor* parentActor,
                                        const al::ActorInitInfo& initInfo);
    static TalkNpcCap* createForShibaken(const al::LiveActor* parentActor,
                                         const al::ActorInitInfo& initInfo);
    static TalkNpcCap* createForShoppingNpc(const al::LiveActor* parentActor,
                                            const al::ActorInitInfo& initInfo);
    static TalkNpcCap* createForShoppingNpcChromakey(const al::LiveActor* parentActor,
                                                     const al::ActorInitInfo& initInfo);
    static TalkNpcCap* createForVolleyballNpc(const al::LiveActor* parentActor,
                                              const al::ActorInitInfo& initInfo);
    void makeActorAlive() override;
    void control() override;
    void init(const al::ActorInitInfo& initInfo) override;

private:
    TalkNpcCap(const CapTypeData* capInfo);

    const CapTypeData* mCapInfo = nullptr;
    const sead::Matrix34f* mBaseMtx = nullptr;
    sead::Vector3f mLocalRotate = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mLocalTrans = {0.0f, 0.0f, 0.0f};
    f32 mScale = 1.0f;
    bool mIsChromakey = false;
};

static_assert(sizeof(TalkNpcCap) == 0x138);
