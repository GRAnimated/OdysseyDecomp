#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class GamePadSystem;
class LayoutInitInfo;
class Scene;
class SimpleLayoutAppearWaitEnd;
class WipeSimple;
}  // namespace al
class FooterParts;
class StageSceneStatePauseMenu;

class StageSceneStateStartSeparatePlay : public al::HostStateBase<StageSceneStatePauseMenu> {
public:
    StageSceneStateStartSeparatePlay(const char*, StageSceneStatePauseMenu*,
                                     const al::LayoutInitInfo&, al::WipeSimple*, al::GamePadSystem*,
                                     FooterParts*);

    virtual void appear();

    void startTreeHouse();
    bool isNeedRequestGraphicsPreset() const;
    bool isDrawViewRenderer() const;
    void exeAppear();
    void exeWait();
    al::Scene* getScene();
    void exeBack();
    void exeFadeOut();
    void exeApplet();
    void exeFadeIn();
    void exeWaitDraw();

    bool getField42() const { return field_42; }

private:
    al::GamePadSystem* mGamePadSystem = nullptr;
    al::SimpleLayoutAppearWaitEnd* mControllerGuideMulti = nullptr;
    al::WipeSimple* mWipeSimple = nullptr;
    FooterParts* mFooterParts = nullptr;
    u16 field_40 = 0;
    bool field_42 = false;
};