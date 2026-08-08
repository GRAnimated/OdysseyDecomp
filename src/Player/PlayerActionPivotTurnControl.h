#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
}
class IUsePlayerCollision;
class IUsePlayerHack;
class PlayerConst;
class PlayerInput;

class PlayerActionPivotTurnControl {
public:
    PlayerActionPivotTurnControl(al::LiveActor* player, const PlayerConst* playerConst,
                                 const PlayerInput* input, const IUsePlayerCollision* collision,
                                 f32 gravity);

    void reset();
    void update();
    void calcMoveDirection(sead::Vector3f* moveDirection, const sead::Vector3f& up);
    void setPlayerHack(IUsePlayerHack** playerHack) { mPlayerHack = playerHack; }
    bool isTurnFinished() const { return _45; }
    bool hasMoveDirection() const { return _44; }

private:
    al::LiveActor* mPlayer;
    const PlayerConst* mConst;
    const PlayerInput* mInput;
    const IUsePlayerCollision* mCollision;
    IUsePlayerHack** mPlayerHack;
    sead::Vector3f _28;
    f32 _34;
    sead::Vector3f _38;
    bool _44;
    bool _45;
    u8 _46[2];
    s32 _48;
    u8 _4c[4];
};

static_assert(sizeof(PlayerActionPivotTurnControl) == 0x50);
