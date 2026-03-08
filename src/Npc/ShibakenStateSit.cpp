#include "Npc/ShibakenStateSit.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorMovementFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/ActorStateReactionBase.h"
#include "Npc/Shibaken.h"
#include "Npc/ShibakenFunction.h"

namespace {
NERVE_IMPL(ShibakenStateSit, SitBow);
NERVE_IMPL(ShibakenStateSit, SitStart);
NERVE_IMPL(ShibakenStateSit, Reaction);
NERVE_IMPL(ShibakenStateSit, Sit);
NERVE_IMPL(ShibakenStateSit, ReactionAfter);
NERVE_IMPL(ShibakenStateSit, SitEnd);

NERVES_MAKE_NOSTRUCT(ShibakenStateSit, SitBow, SitStart, Reaction, Sit, ReactionAfter, SitEnd);
}  // namespace

ShibakenStateSit::ShibakenStateSit(const char* name, Shibaken* shibaken,
                                   ActorStateReactionBase* reaction, bool alwaysSit)
    : HostStateBase<Shibaken>(name, shibaken), mReaction(reaction), mAlwaysSit(alwaysSit) {
    initNerve(&SitStart, mReaction != nullptr);
    if (mReaction)
        al::initNerveState(this, mReaction, &Reaction, u8"リアクション");
}

void ShibakenStateSit::appear() {
    NerveStateBase::appear();
    if (mAlwaysSit) {
        mAlwaysSit = false;
        al::setNerve(this, &Sit);
    } else {
        al::setNerve(this, &SitStart);
    }
}

void ShibakenStateSit::tryStartSitEnd() {
    if (al::isNerve(this, &ReactionAfter)) {
        kill();
        return;
    }
    if (al::isNerve(this, &Sit))
        al::setNerve(this, &SitEnd);
}

bool ShibakenStateSit::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                                  al::HitSensor* self) {
    return mReaction &&
           ShibakenFunction::tryStartReaction(this, mReaction, &Reaction, msg, other, self);
}

void ShibakenStateSit::exeSitStart() {
    if (al::isFirstStep(this))
        al::startAction(getHost(), "SitStart");
    if (al::isExistActorCollider(getHost())) {
        ShibakenFunction::addFallVelocityToGround(getHost(), -1.0f);
        ShibakenFunction::limitFallVelocityOnGround(getHost());
    } else {
        al::setVelocityZero(getHost());
    }
    if (al::isActionEnd(getHost()))
        al::setNerve(this, &Sit);
}

void ShibakenStateSit::exeSit() {
    if (al::isFirstStep(this))
        al::startAction(getHost(), "Sit");
    if (al::isExistActorCollider(getHost())) {
        ShibakenFunction::addFallVelocityToGround(getHost(), -1.0f);
        ShibakenFunction::limitFallVelocityOnGround(getHost());
    } else {
        al::setVelocityZero(getHost());
    }
    if (al::isGreaterEqualStep(this, 120)) {
        if (al::calcNerveRate(this, 120, 360) < al::getRandom())
            al::setNerve(this, &SitBow);
    }
}

void ShibakenStateSit::exeSitBow() {
    if (al::isFirstStep(this))
        al::startAction(getHost(), "SitBow");
    if (al::isExistActorCollider(getHost())) {
        ShibakenFunction::addFallVelocityToGround(getHost(), -1.0f);
        ShibakenFunction::limitFallVelocityOnGround(getHost());
    } else {
        al::setVelocityZero(getHost());
    }
    if (al::isActionEnd(getHost()))
        al::setNerve(this, &Sit);
}

void ShibakenStateSit::exeSitEnd() {
    if (al::isFirstStep(this))
        al::startAction(getHost(), "SitEnd");
    if (al::isExistActorCollider(getHost())) {
        ShibakenFunction::addFallVelocityToGround(getHost(), -1.0f);
        ShibakenFunction::limitFallVelocityOnGround(getHost());
    } else {
        al::setVelocityZero(getHost());
    }
    if (al::isActionEnd(getHost()))
        kill();
}

void ShibakenStateSit::exeReaction() {
    if (ShibakenFunction::executeReactionNerve(this))
        al::setNerve(this, &ReactionAfter);
}

void ShibakenStateSit::exeReactionAfter() {
    if (al::isFirstStep(this))
        al::startAction(getHost(), "Wait");
    al::setNerveAtGreaterEqualStep(this, &SitStart, 60);
}
