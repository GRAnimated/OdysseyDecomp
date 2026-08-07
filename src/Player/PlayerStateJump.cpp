#include "Player/PlayerStateJump.h"

#include "Library/Base/StringUtil.h"

bool PlayerStateJump::isJumpSpinGroundClockwise() const {
    return al::isEqualString(_c8, "SpinJumpR");
}
