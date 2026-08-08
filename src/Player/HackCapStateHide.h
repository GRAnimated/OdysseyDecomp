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
    HackCapStateHide(al::LiveActor* actor, PlayerColliderHackCap* collider,
                     const al::LiveActor* player, const PlayerSeparateCapFlag* separateCapFlag,
                     const PlayerInput* input, al::PartsModel* partsModel,
                     HackCapJointControlKeeper* jointControlKeeper);
    void appear() override;

    void kill() override;
    bool update() override;
    bool isSeparateMode() const;
    void cancelSeparateMode();
    void calcSeparateThrowOffset(sead::Vector3f* offset) const;
    void exeHide();
    void tryForceFollowSeparate();
    void exeSeparateWait();

    void requestForceFollowSeparate() { _64 = true; }

    ~HackCapStateHide() override;

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
