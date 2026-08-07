#include "Player/PlayerInputFunction.h"

#include "Library/Controller/InputFunction.h"
#include "Library/LiveActor/LiveActor.h"

bool PlayerInputFunction::isTriggerAction(const al::LiveActor* actor, s32 port) {
    if (rs::isSeparatePlay(actor) && al::isPadTypeJoySingle(port))
        return al::isPadTriggerY(port);
    return al::isPadTriggerX(port) || al::isPadTriggerY(port);
}

bool PlayerInputFunction::isTriggerTalk(const al::LiveActor*, s32 port) {
    return al::isPadTriggerA(port);
}

bool PlayerInputFunction::isTriggerStartWorldWarp(const al::LiveActor*, s32 port) {
    return al::isPadTriggerA(port);
}
