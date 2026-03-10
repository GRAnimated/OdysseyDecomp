#pragma once

#include "Library/Nerve/NerveStateBase.h"

class Shibaken;

class ShibakenStateWait : public al::NerveStateBase {
public:
    ShibakenStateWait(const char* name, Shibaken* shibaken, bool isAlwaysWait);

    void appear() override;
    bool isPlayingWait() const;
    void exeWait();
    void exeBow();
    void exeShake();
    void exeJump();

private:
    Shibaken* mShibaken = nullptr;
    s32 mWaitFrames = 120;
    bool mIsAlwaysWait = false;
};

static_assert(sizeof(ShibakenStateWait) == 0x28);
