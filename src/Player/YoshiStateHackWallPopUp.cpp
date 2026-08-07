#include "Player/YoshiStateHackWallPopUp.h"

#include "Library/Collision/PartsConnector.h"
#include "Library/Collision/PartsConnectorUtil.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/ParabolicPath.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
NERVE_IMPL(YoshiStateHackWallPopUp, PopUp)
NERVES_MAKE_NOSTRUCT(YoshiStateHackWallPopUp, PopUp)
}  // namespace

YoshiStateHackWallPopUp::YoshiStateHackWallPopUp(al::LiveActor* actor,
                                                 IUsePlayerHack** playerHack,
                                                 const PlayerConst* playerConst,
                                                 const IUsePlayerCollision* collision,
                                                 PlayerAnimator* animator)
    : HackerStateBase("壁はね上がり", actor, playerHack), mPlayerConst(playerConst),
      mCollision(collision), mAnimator(animator), mStartTrans(0.0f, 0.0f, 0.0f),
      mStartQuat(sead::Quatf::unit), mCollisionParts(nullptr), mConnector(nullptr),
      mSnapMtx(sead::Matrix34f::ident), mConnectedMtx(sead::Matrix34f::ident), mPath(nullptr) {
    mConnector = al::createCollisionPartsConnector(actor, sead::Quatf::unit);
    mPath = new al::ParabolicPath();
    initNerve(&PopUp, 0);
}

void YoshiStateHackWallPopUp::appear() {
    al::LiveActor* actor = mActor;
    HackerStateBase::appear();
    mStartTrans = al::getTrans(actor);
    al::calcQuat(&mStartQuat, actor);
    al::setVelocityZero(actor);
    al::setNerve(this, &PopUp);
}

void YoshiStateHackWallPopUp::kill() {
    mCollisionParts = nullptr;
    al::disconnectMtxConnector(mConnector);
    HackerStateBase::kill();
}
