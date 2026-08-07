#include "Player/HackerStateBase.h"

HackerStateBase::HackerStateBase(const char* name, al::LiveActor* actor,
                                 IUsePlayerHack** playerHack)
    : al::NerveStateBase(name), mActor(actor), mPlayerHack(playerHack) {}

