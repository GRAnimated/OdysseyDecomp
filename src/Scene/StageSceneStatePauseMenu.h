#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class ActorInitInfo;
class ApplicationMessageReceiver;
class KeyRepeatCtrl;
class LayoutInitInfo;
class PauseCameraCtrl;
class Scene;
class SceneInitInfo;
class SimpleLayoutAppearWaitEnd;
class WindowConfirm;
class WipeSimple;
}  // namespace al
class FooterParts;
class GameDataHolder;
class MenuSelectParts;
class SceneAudioSystemPauseController;
class StageSceneLayout;
class StageSceneStateEndSeparatePlay;
class StageSceneStateOption;
class StageSceneStateStartSeparatePlay;

class StageSceneStatePauseMenu : public al::HostStateBase<al::Scene> {
public:
    StageSceneStatePauseMenu(const char*, al::Scene*, al::SimpleLayoutAppearWaitEnd*,
                             GameDataHolder*, const al::SceneInitInfo&, const al::ActorInitInfo&,
                             const al::LayoutInitInfo&, al::WindowConfirm*, StageSceneLayout*, bool,
                             SceneAudioSystemPauseController*);

    virtual void appear();
    virtual void kill();

    void killPauseMenu();
    void killMarioModel();
    bool isNeedKillHost() const;
    void startNormal();
    void startAfterTitle();
    void killAllOptionLayout();
    bool isEndToCancel() const;
    bool isEndToHelp() const;
    bool isLoadData() const;
    void getSelectedFileId() const;
    bool isChangeLanguage() const;
    void getLanguage() const;
    bool isNewGame() const;
    bool isModeSelectEnd() const;
    void checkNeedKillByHostAndEnd();
    void startActionMario(char const*);
    void getMarioActor() const;
    bool isDrawLayout() const;
    bool isDrawLayoutMain() const;
    bool isDrawViewRenderer() const;
    bool isDrawChromakey() const;
    void exeAppear();
    void setNormal();
    void appearMarioModel();
    void updatePlayerPose();
    void exeWait();
    void changeNerveAndReturn(const al::Nerve* nerve);
    void exeFadeBeforeHelp();
    void exeStartHelp();
    void exeWaitDraw();
    void exeEnd();
    void exeStartSeparatePlay();
    void exeEndSeparatePlay();
    void exeOption();
    void exeSave();
    void exeConfirmNewGame();
    void exeNotExistEmptyFile();
    void startPauseCamera();
    void setAfterTitle();

private:
    al::SimpleLayoutAppearWaitEnd* mMenuLayout;
    al::SimpleLayoutAppearWaitEnd* mMenuGuide;
    al::SimpleLayoutAppearWaitEnd* mMenuRight;
    FooterParts* mFooterParts;
    MenuSelectParts* mSelectParts;
    al::WipeSimple* mMenuWipe;
    al::WipeSimple* mHelpWipe;
    int mStartType;
    int field_0x5c;
    StageSceneStateStartSeparatePlay* mStateStartSeparatePlay;
    StageSceneStateEndSeparatePlay* mStateEndSeparatePlay;
    StageSceneStateOption* mStateOption;
    al::LiveActor* mMarioHigh;
    char field_0x80[40];
    GameDataHolder* mGameDataHolder;
    al::PauseCameraCtrl* mPauseCameraCtrl;
    al::WindowConfirm* mWindowConfirm;
    bool mIsNewGame;
    al::KeyRepeatCtrl* mKeyRepeatCtrl;
    StageSceneLayout* mStageSceneLayout;
    bool mIsNormal;
    bool mIsPauseMenu;
    float mPrevNearClipDistance;
    SceneAudioSystemPauseController* mSceneAudioSystemPauseController;
    al::ApplicationMessageReceiver* mApplicationMessageReceiver;
};