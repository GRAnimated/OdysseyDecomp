#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
}  // namespace al
class IPlayerModelChanger;
class IUseDimension;
class IUsePlayerCollision;
class PlayerAnimator;
class PlayerConst;
class PlayerInput;
class PlayerStateRun;
class PlayerStateRun2D;

class PlayerStateRun2D3D : public al::ActorStateBase {
public:
    PlayerStateRun2D3D(al::LiveActor* player, const PlayerConst* pConst,
                       const IUseDimension* dimension, const IPlayerModelChanger* modelChanger,
                       const PlayerInput* input, const IUsePlayerCollision* collision,
                       PlayerAnimator* animator);
    void appear() override;

    void syncModel();
    void exeRun3D();
    void exeRun2D();
    ~PlayerStateRun2D3D() override;

private:
    const PlayerConst* mConst;
    const IPlayerModelChanger* mModelChanger;
    PlayerStateRun* mRun3D = nullptr;
    PlayerStateRun2D* mRun2D = nullptr;
};

