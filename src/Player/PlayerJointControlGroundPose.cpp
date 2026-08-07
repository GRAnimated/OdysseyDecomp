#include "Player/PlayerJointControlGroundPose.h"

f32 PlayerJointControlGroundPose::initCenterBalanceRate() const {
    return 1.0f;
}

f32 PlayerJointControlGroundPose::calcCenterBalanceBlendRate() const {
    return _220 * _1dc;
}

void PlayerJointControlGroundPose::resetTiltRate() {
    _1e4 = 0.0f;
    _1e8 = 0.0f;
    _228 = 0.0f;
    _22c = 0.0f;
    _230 = 0.0f;
}

const char* PlayerJointControlGroundPose::getCtrlTypeName() const {
    return "地上姿勢制御";
}
