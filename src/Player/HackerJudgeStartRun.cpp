#include "Player/HackerJudgeStartRun.h"

#include "Library/LiveActor/ActorCollisionFunction.h"

#include "Util/PlayerCollisionUtil.h"
#include "Util/PlayerHackInputFunction.h"

HackerJudgeStartRun::HackerJudgeStartRun(const al::LiveActor* parent, IUsePlayerHack** hacker)
    : HackerJudge(hacker), mParent(parent), _18(nullptr), mPlayerCollision(nullptr), _28(0.0f) {}

bool HackerJudgeStartRun::judge() const {
    if (mPlayerCollision) {
        if (!rs::isCollidedGround(mPlayerCollision))
            return false;
    } else if (!al::isCollidedGround(mParent)) {
        return false;
    }

    if (rs::isOnHackMoveStick(*getHacker()))
        return true;

    if (_18 && *_18 > 0)
        return true;

    if (mPlayerCollision && _28 > 0.0f &&
        rs::isAutoRunOnGroundSkateCode(mParent, mPlayerCollision, _28))
        return true;

    return false;
}

void HackerJudgeStartRun::reset() {}

void HackerJudgeStartRun::update() {}
