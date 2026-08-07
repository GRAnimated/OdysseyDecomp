#include "Player/PlayerStateSpinCap.h"

#include "Player/PlayerJointParamCapThrow.h"

bool PlayerStateSpinCap::noticeInWater() {
    if (_98)
        return false;

    _98 = true;
    return true;
}

void PlayerStateSpinCap::resetJoint() {
    mCapThrowJoint->isEnd = true;
}
