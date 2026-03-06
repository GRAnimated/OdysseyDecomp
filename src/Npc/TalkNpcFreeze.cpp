#include "Npc/TalkNpcFreeze.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"

TalkNpcFreeze::TalkNpcFreeze(const char* name) : al::LiveActor(name) {}

void TalkNpcFreeze::init(const al::ActorInitInfo& info) {
    al::initActorChangeModelSuffix(this, info, "Freeze");
    al::startAction(this, "Freeze");

    if (al::isSklAnimPlaying(this, 0))
        al::setSklAnimFrameRate(this, 0.0f, 0);
    if (al::isMtpAnimPlaying(this))
        al::setMtpAnimFrameRate(this, 0.0f);
    if (al::isMclAnimPlaying(this))
        al::setMclAnimFrameRate(this, 0.0f);
    if (al::isMtsAnimPlaying(this))
        al::setMtsAnimFrameRate(this, 0.0f);
    if (al::isVisAnimPlaying(this))
        al::setVisAnimFrameRate(this, 0.0f);

    if (al::isVisAnimExist(this, "Off"))
        al::startVisAnimAndSetFrameAndStop(this, "Off", 0.0f);

    al::trySyncStageSwitchAppearAndKill(this);
}

void TalkNpcFreeze::makeActorAlive() {
    al::LiveActor::makeActorAlive();
    al::LiveActor::calcAnim();
}

void TalkNpcFreeze::movement() {}

void TalkNpcFreeze::calcAnim() {}
