#pragma once

#include "Library/Scene/Scene.h"

namespace al {
class ChromakeyDrawer;
class SimpleLayoutAppearWaitEnd;
class WindowConfirm;
class WipeSimple;
}  // namespace al
class StageSceneStatePauseMenu;
class BootLayout;

class TitleMenuScene : public al::Scene {
public:
    TitleMenuScene();
    ~TitleMenuScene();
    ;

    void init(al::SceneInitInfo const& info);
    void appear();
    void control();
    void drawMain() const;
    bool isEnableKill() const;
    bool isChangeLanguage() const;
    const char* getLanguage() const;
    bool isNewGame() const;
    void startLoadDirect(bool);
    void setScenario();
    void setLoadPercent(f32 percent);
    bool isCancelLoadWorldResource() const;
    void startLoadWorldResource();
    void exeAppear();
    void exeMenu();
    void exeWipe();
    void exeLoadAppear();
    void exeLoadWait();

private:
    StageSceneStatePauseMenu* mStatePauseMenu = nullptr;
    al::ChromakeyDrawer* mChromakeyDrawer = nullptr;
    al::SimpleLayoutAppearWaitEnd* mMenuLayout = nullptr;
    al::SimpleLayoutAppearWaitEnd* mLayoutContinueLoading = nullptr;
    al::SimpleLayoutAppearWaitEnd* mLayoutParBG = nullptr;
    al::WindowConfirm* mWindowConfirm = nullptr;
    al::WipeSimple* mFadeBlack = nullptr;
    BootLayout* mBootLayout = nullptr;
    float mLoadPercent = 0.0f;
    float float_11c = 0.0f;
    bool mIsCancelLoadResource = false;
    int mEnableKillStep = 0;
};
