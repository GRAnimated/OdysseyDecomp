#pragma once

#include "Library/Layout/LayoutActor.h"

namespace al {
class LayoutActor;
class LayoutInitInfo;
}  // namespace al

class DecideIconLayoutParts : public al::LayoutActor {
public:
    DecideIconLayoutParts(const char* name, al::LayoutActor* parentActor,
                          const al::LayoutInitInfo& initInfo);

    void start();

    void exeAppear();
    void exeWait();
    void exeDecide();
    void exeDecideAfter();
    void exeEnd();

    bool isDecide() const;
    bool isWait() const;
    bool isEnd() const;
};

static_assert(sizeof(DecideIconLayoutParts) == 0x130);
