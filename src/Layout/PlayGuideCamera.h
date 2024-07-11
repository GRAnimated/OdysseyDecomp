#pragma once

#include "Library/Layout/LayoutActor.h"

namespace al {
class LayoutInitInfo;
class LiveActor;
}  // namespace al

class PlayGuideCamera : public al::LayoutActor {
public:
    PlayGuideCamera(const char*, const al::LayoutInitInfo&, const al::LiveActor*);

    void hide();
    void start();

    void exeHide();
    bool tryAppear();
    void exeAppear();
    void exeWait();
    void exeEnd();

private:
    bool field_129 = false;
    const al::LiveActor* field_130 = nullptr;
};
