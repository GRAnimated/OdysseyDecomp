#include "Npc/TalkNpcSwitchWaitActionObj.h"

#include "Library/LiveActor/ActorInitInfo.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Placement/PlacementInfo.h"
#include "Library/Stage/StageSwitchKeeper.h"
#include "Library/Stage/StageSwitchUtil.h"
#include "Library/Thread/FunctorV0M.h"

#include "Npc/RandomActionUpdater.h"
#include "Npc/TalkNpcActionAnimInfo.h"

TalkNpcSwitchWaitActionObj* TalkNpcSwitchWaitActionObj::tryCreate(
    const TalkNpcParam* param, TalkNpcActionAnimInfo* animInfo, RandomActionUpdater* updater,
    const al::ActorInitInfo& initInfo) {
    al::PlacementInfo placementInfo;
    if (!al::tryGetLinksInfo(&placementInfo, initInfo, "SwitchWaitActionObj"))
        return nullptr;

    return new TalkNpcSwitchWaitActionObj(initInfo.stageSwitchDirector, param, animInfo, updater,
                                          placementInfo);
}

TalkNpcSwitchWaitActionObj::TalkNpcSwitchWaitActionObj(
    al::StageSwitchDirector* switchDirector, const TalkNpcParam* param,
    TalkNpcActionAnimInfo* animInfo, RandomActionUpdater* updater,
    const al::PlacementInfo& placementInfo)
    : mParam(param), mAnimInfo(animInfo), mActionUpdater(updater) {
    al::initStageSwitch(this, switchDirector, placementInfo);
    al::FunctorV0M<TalkNpcSwitchWaitActionObj*, void (TalkNpcSwitchWaitActionObj::*)()> onFunctor(
        this, &TalkNpcSwitchWaitActionObj::listenStageSwitchStartWaitActionSwitchOn);
    al::FunctorV0M<TalkNpcSwitchWaitActionObj*, void (TalkNpcSwitchWaitActionObj::*)()> offFunctor(
        this, &TalkNpcSwitchWaitActionObj::listenStageSwitchStartWaitActionSwitchOff);
    al::listenStageSwitchOnOff(this, "SwitchWaitActionSwitchStart", onFunctor, offFunctor);
    al::getStringArg(&mWaitActionName, placementInfo, "WaitActionName");
}

void TalkNpcSwitchWaitActionObj::listenStageSwitchStartWaitActionSwitchOn() {
    if (mAnimInfo->changeWaitActionNameBySwitch(mWaitActionName, mParam))
        mActionUpdater->tryStartWaitActionIfNotPlaying();
}

void TalkNpcSwitchWaitActionObj::listenStageSwitchStartWaitActionSwitchOff() {
    if (mAnimInfo->resetWaitActionNameBySwitch(mParam))
        mActionUpdater->tryStartWaitActionIfNotPlaying();
}

void TalkNpcSwitchWaitActionObj::initStageSwitchKeeper() {
    mStageSwitchKeeper = new al::StageSwitchKeeper();
}
