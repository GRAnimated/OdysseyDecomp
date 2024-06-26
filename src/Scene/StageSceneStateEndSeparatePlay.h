#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class GamePadSystem;
class LayoutInitInfo;
class Scene;
class WipeSimple;
}  // namespace al
class StageSceneStatePauseMenu;

class StageSceneStateEndSeparatePlay : public al::HostStateBase<StageSceneStatePauseMenu> {
public:
    StageSceneStateEndSeparatePlay(char const*, StageSceneStatePauseMenu*,
                                   al::LayoutInitInfo const&, al::WipeSimple*, al::GamePadSystem*);

    virtual void appear();

    bool isNeedRequestGraphicsPreset() const;
    void exeFadeOut();
    void exeApplet();
    al::Scene* getScene();
    void exeFadeIn();
    void exeWaitDraw();
    bool isDrawViewRenderer() const;

private:
    al::GamePadSystem* mGamePadSystem = nullptr;
    al::WipeSimple* mWipeSimple = nullptr;
    bool field_30 = false;
};