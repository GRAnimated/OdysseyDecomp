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
    PlayerActionPivotTurnControl(al::LiveActor*, const PlayerConst*, const PlayerInput*,
                                 const IUsePlayerCollision*, f32);

    void reset();
    void update();
    void calcMoveDirection(sead::Vector3f*, const sead::Vector3f&);

private:
    al::LiveActor* mPlayer;
    const PlayerConst* mConst;
    const PlayerInput* mInput;
    const IUsePlayerCollision* mCollision;
    const IUsePlayerHack* const* mPlayerHack;
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
