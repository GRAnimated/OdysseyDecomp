#pragma once

#include "Library/Nerve/NerveExecutor.h"

namespace nn::ui2d {
class TextureInfo;
}
namespace al {
class LayoutActor;
class LayoutInitInfo;
class SimpleLayoutAppearWaitEnd;
}  // namespace al

class WindowConfirmData : public al::NerveExecutor {
public:
    enum PaneType : s32 { PaneType_Confirm = 0, PaneType_Cancel = 1 };

    WindowConfirmData(const al::LayoutInitInfo&, const char*, const char*, bool);

    void setConfirmMessage(const char16*, const char16*, const char16*);
    void setConfirmData(al::LayoutActor* actor, nn::ui2d::TextureInfo* textureInfo);
    void updateConfirmDataDate();
    void appear();
    void appearWithChoicingCancel();
    void end();
    void kill();
    bool isEndSelect();
    bool isDecided();
    bool isCanceled();
    bool isDisable();
    void exeAppear();
    void changeSelectingIdx(s32 index);
    void exeWait();
    void updateCursorPos();
    void exeSelect();
    void exeVanish();
    void exeDisable();

    al::LayoutActor* getWindowConfirm(PaneType type) const { return *(&mParConfirm + type); }

private:
    al::SimpleLayoutAppearWaitEnd* mWindowConfirmLayout = nullptr;
    al::LayoutActor* mParCursor = nullptr;
    al::LayoutActor* mParConfirm = nullptr;
    al::LayoutActor* mParCancel = nullptr;
    al::LayoutActor* mParData = nullptr;
    PaneType mPaneIndex = PaneType::PaneType_Confirm;
    int field_3C = 0;
};