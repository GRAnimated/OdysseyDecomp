#pragma once

#include "Library/Nerve/NerveExecutor.h"

struct ActorInitInfo;
class JumpingRopeNpc;

class JumpingRopeLayout : public al::NerveExecutor {
public:
    JumpingRopeLayout(JumpingRopeNpc* npc, const al::ActorInitInfo& info);
    ~JumpingRopeLayout() override;

    void exeWait();
    void exeGame();
    void exeResult();
    void exeEnd();
    void exeDisplayTutorial();

    bool isWaiting() const;
    void setBest(u32 best);
};
