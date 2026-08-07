#include "Player/PlayerStateRunHakoniwa2D3D.h"

#include "Player/PlayerStateGroundSpin.h"

bool PlayerStateRunHakoniwa2D3D::isSpinClockwise() const {
    return mGroundSpin3D->isSpinClockwise();
}
