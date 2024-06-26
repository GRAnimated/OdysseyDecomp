#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class GamePadSystem;
class LayoutInitInfo;
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

    void appear();
    void startTreeHouse();
    bool isNeedRequestGraphicsPreset() const;
    bool isDrawViewRenderer() const;
    void exeAppear();
    void exeWait();
    void getScene();
    void exeBack();
    void exeFadeOut();
    void exeApplet();
    void exeFadeIn();
    void exeWaitDraw();

private:
    al::GamePadSystem* mGamePadSystem;
    al::SimpleLayoutAppearWaitEnd* mControllerGuideMulti;
    al::WipeSimple* mWipeSimple;
    FooterParts* mFooterParts;
    u16 field_40 = 0;
    bool field_42 = false;
};