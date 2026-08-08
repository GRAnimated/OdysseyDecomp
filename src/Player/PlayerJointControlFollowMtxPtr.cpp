#include "Player/PlayerJointControlFollowMtxPtr.h"

PlayerJointControlFollowMtxPtr::PlayerJointControlFollowMtxPtr(const sead::Matrix34f* followMtx)
    : al::JointControllerBase(16), mIsValid(true), mFollowMtx(followMtx) {}

void PlayerJointControlFollowMtxPtr::calcJointCallback(s32, sead::Matrix34f* jointMtx) {
    if (mIsValid)
        *jointMtx = *mFollowMtx;
}

const char* PlayerJointControlFollowMtxPtr::getCtrlTypeName() const {
    return "ジョイントの行列指定バージョン";
}
