#pragma once

#include "Library/Nerve/NerveExecutor.h"

namespace al {
class LayoutInitInfo;
class SimpleLayoutAppearWaitEnd;
}  // namespace al

class PlayGuideMap : public al::NerveExecutor {
public:
    PlayGuideMap(const char*, const al::LayoutInitInfo&);
    void start();
    void end();
    void endImmediate();
    void exeHide();
    void exeShow();
    void exeEnd();

private:
    al::SimpleLayoutAppearWaitEnd* mLayout = nullptr;
};