#pragma once

#include "Library/Nerve/NerveExecutor.h"

namespace al {
struct ActorInitInfo;
}

class JumpingRopeNpc;
class MiniGameCueLayout;
class MiniGameLayout;

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

private:
    friend class JumpingRopeNpc;

    JumpingRopeNpc* mNpc = nullptr;
    MiniGameLayout* mMiniGameLayout = nullptr;
    MiniGameCueLayout* mCueLayout = nullptr;
};
