#include "Player/PlayerSandSinkAffect.h"

bool PlayerSandSinkAffect::isSink() const {
    return mSinkVelocity > 0.0f;
}
