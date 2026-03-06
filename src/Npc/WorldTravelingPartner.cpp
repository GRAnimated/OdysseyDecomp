#include "Npc/WorldTravelingPartner.h"

#include "Library/Event/EventFlowUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorSensorFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/NpcStateReaction.h"
#include "Npc/TalkNpcParam.h"
#include "Util/NpcAnimUtil.h"
#include "Util/NpcEventFlowUtil.h"

namespace {
NERVE_IMPL(WorldTravelingPartner, Wait);
NERVE_IMPL(WorldTravelingPartner, Talk);
NERVE_IMPL(WorldTravelingPartner, Scared);
NERVE_IMPL(WorldTravelingPartner, Reaction);
NERVES_MAKE_NOSTRUCT(WorldTravelingPartner, Wait, Talk, Scared, Reaction);
}  // namespace

WorldTravelingPartner::WorldTravelingPartner(const char* name) : al::LiveActor(name) {}

void WorldTravelingPartner::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, sead::SafeString("CityMan"), nullptr);
    al::initNerve(this, &Wait, 1);
    mTalkNpcParam = rs::initTalkNpcParam(this, nullptr);
    NpcStateReaction* reaction = NpcStateReaction::createForHuman(this, nullptr);
    mNpcStateReaction = reaction;
    al::initNerveState(this, reaction, &Reaction, u8"リアクション");
    rs::setNpcMaterialAnimFromPlacementInfo(this, info);
    makeActorAlive();
}

void WorldTravelingPartner::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    rs::attackSensorNpcCommon(self, other);
}

bool WorldTravelingPartner::receiveMsg(const al::SensorMsg* msg, al::HitSensor* other,
                                       al::HitSensor* self) {
    if (rs::isMsgPlayerDisregardHomingAttack(msg))
        return true;
    if (al::isMsgPlayerDisregard(msg))
        return mTalkNpcParam->isPlayerWatchDisregard(self);

    if (mNpcStateReaction != nullptr) {
        if (mNpcStateReaction->receiveMsg(msg, other, self)) {
            al::setNerve(this, &Reaction);
            return true;
        }
        if (mNpcStateReaction->receiveMsgNoReaction(msg, other, self))
            return true;
    }
    return false;
}

void WorldTravelingPartner::startWait() {
    if (!al::isNerve(this, &Wait))
        al::setNerve(this, &Wait);
}

void WorldTravelingPartner::startTalk() {
    if (!al::isNerve(this, &Talk))
        al::setNerve(this, &Talk);
}

void WorldTravelingPartner::exeWait() {
    if (al::isFirstStep(this))
        al::startAction(this, "Wait");
    if (al::isScare(mEventFlowData))
        al::setNerve(this, &Scared);
}

void WorldTravelingPartner::exeTalk() {
    if (al::isFirstStep(this))
        al::startAction(this, "Talk");
}

void WorldTravelingPartner::exeScared() {
    if (al::isFirstStep(this))
        al::startAction(this, "Scared");
    if (!al::isScare(mEventFlowData))
        al::setNerve(this, &Wait);
}

void WorldTravelingPartner::exeReaction() {
    al::updateNerveStateAndNextNerve(this, &Wait);
}

void WorldTravelingPartner::initAfterPlacement() {}
