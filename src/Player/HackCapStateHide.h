#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
class PartsModel;
}  // namespace al
class HackCapJointControlKeeper;
class PlayerColliderHackCap;
class PlayerInput;
class PlayerSeparateCapFlag;

class HackCapStateHide : public al::ActorStateBase {
public:
    HackCapStateHide(al::LiveActor*, PlayerColliderHackCap*, const al::LiveActor*,
                     const PlayerSeparateCapFlag*, const PlayerInput*, al::PartsModel*,
                     HackCapJointControlKeeper*);
    ~HackCapStateHide() override;

    void appear() override;
    void kill() override;
    bool update() override;
    bool isSeparateMode() const;
    void cancelSeparateMode();
    void calcSeparateThrowOffset(sead::Vector3f*) const;
    void exeHide();
    void tryForceFollowSeparate();

    void requestForceFollowSeparate() { _64 = true; }

    void exeSeparateWait();

private:
    PlayerColliderHackCap* mCollider;
    const al::LiveActor* mPlayer;
    const PlayerSeparateCapFlag* mSeparateCapFlag;
    const PlayerInput* mInput;
    al::PartsModel* mPartsModel;
    al::PartsModel* mSubActor;
    HackCapJointControlKeeper* mJointControlKeeper;
    sead::Vector3f _58;
    bool _64;
    u8 _65[3];
    sead::Vector3f _68;
    u8 _74[4];
};

static_assert(sizeof(HackCapStateHide) == 0x78);
