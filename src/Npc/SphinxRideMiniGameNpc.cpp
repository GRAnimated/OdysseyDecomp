#include "Npc/SphinxRideMiniGameNpc.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Util/DemoUtil.h"
#include "Util/NpcEventFlowUtil.h"

namespace {
NERVE_IMPL(SphinxRideMiniGameNpc, TalkWait);
NERVE_IMPL(SphinxRideMiniGameNpc, TalkEnd);
NERVES_MAKE_NOSTRUCT(SphinxRideMiniGameNpc, TalkWait, TalkEnd);
}  // namespace

SphinxRideMiniGameNpc::SphinxRideMiniGameNpc(const char* name)
    : al::LiveActor("乗れるスフィンクス[ミニゲームNPC]") {}

void SphinxRideMiniGameNpc::init(const al::ActorInitInfo& info) {
    al::initActorWithArchiveName(this, info, "DesertMan", nullptr);
    al::initNerve(this, &TalkWait, 0);
    mEventFlowExecutor =
        rs::initEventFlow(this, info, "SphinxRideMiniGameNpc", nullptr);

    auto* displayModel =
        new al::LiveActor("乗れるスフィンクス(ミニゲームNPCのディスプレイ)");
    mDisplayModel = displayModel;
    al::initActorWithArchiveName(displayModel, info, "SphinxRide", nullptr);
    al::startAction(mDisplayModel, "Wait");

    al::LiveActor* model = mDisplayModel;
    const sead::Vector3f& trans = al::getTrans(this);
    sead::Vector3f pos = {trans.x + 170.0f, trans.y + 0.0f, trans.z + 0.0f};
    al::setTrans(model, pos);

    mDisplayModel->makeActorAlive();
    makeActorAlive();
}

void SphinxRideMiniGameNpc::control() {}

void SphinxRideMiniGameNpc::exeTalkWait() {
    if (al::isFirstStep(this))
        al::startAction(this, "Wait");
}

void SphinxRideMiniGameNpc::exeTalkEnd() {}

bool SphinxRideMiniGameNpc::isNerveTalkEnd() const {
    return al::isNerve(this, &TalkEnd);
}

void SphinxRideMiniGameNpc::hideModel() {
    al::hideModel(this);
    al::hideModel(mDisplayModel);
}

void SphinxRideMiniGameNpc::showModelAndAddDemo() {
    al::showModel(this);
    al::showModel(mDisplayModel);
    rs::addDemoActor(this, false);
    rs::addDemoActor(mDisplayModel, false);
}
