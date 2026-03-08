#include "Npc/RandomWaitActionUpdater.h"

#include "Npc/RandomActionUpdater.h"
#include "Npc/TalkNpcActionAnimInfo.h"

RandomWaitActionUpdater::RandomWaitActionUpdater(al::LiveActor* actor,
                                                 const al::ActorInitInfo& info,
                                                 const TalkNpcParam* param, const char* waitAction,
                                                 const char* byeAction) {
    mActionAnimInfo = new TalkNpcActionAnimInfo();
    mActionAnimInfo->mWaitActionName = waitAction;
    mActionAnimInfo->init(actor, info, param, nullptr);
    mRandomActionUpdater = new RandomActionUpdater(actor, mActionAnimInfo);
    mRandomActionUpdater->_a0 = 0.5f;
    if (byeAction)
        mRandomActionUpdater->initBalloonAction(byeAction);
}

void RandomWaitActionUpdater::setDisableBalloonAction() {
    mRandomActionUpdater->_9c = 120;
}

void RandomWaitActionUpdater::setRandomOutbreakProbability(f32 probability) {
    mRandomActionUpdater->_a0 = probability;
}

void RandomWaitActionUpdater::update() {
    mRandomActionUpdater->update();
}
