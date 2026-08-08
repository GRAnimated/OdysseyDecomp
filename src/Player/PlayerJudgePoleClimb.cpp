#include "Player/PlayerJudgePoleClimb.h"

PlayerJudgePoleClimb::PlayerJudgePoleClimb(
    const al::LiveActor* player, const PlayerConst* pConst, const IUsePlayerCollision* collision,
    const IPlayerModelChanger* modelChanger, const PlayerCarryKeeper* carryKeeper,
    const PlayerExternalVelocity* externalVelocity, const PlayerInput* input,
    const PlayerTrigger* trigger)
    : mPlayer(player), mConst(pConst), mCollision(collision), mModelChanger(modelChanger),
      mCarryKeeper(carryKeeper), mExternalVelocity(externalVelocity), mInput(input),
      mTrigger(trigger) {}

void PlayerJudgePoleClimb::reset() {
    mIsJudge = false;
    mPoleHeight = 0.0f;
    mPosition.set(0.0f, 0.0f, 0.0f);
    mUp.set(0.0f, 0.0f, 0.0f);
    mFront.set(0.0f, 0.0f, 0.0f);
    mAngleOffsetWall = 0.0f;
}


bool PlayerJudgePoleClimb::judge() const {
    return mIsJudge;
}
