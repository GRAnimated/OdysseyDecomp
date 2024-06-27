#pragma once

#include "Library/Layout/LayoutActor.h"

#include <container/seadPtrArray.h>

namespace al {
class LayoutInitInfo;

class WindowConfirm : public al::LayoutActor {
public:
    WindowConfirm(const al::LayoutInitInfo&, const char*, const char*);

    void setTxtMessage(const char16*);
    void setTxtList(s32, const char16*);
    void setListNum(s32);
    void setCancelIdx(s32);
    void appear();
    void appearWithChoicingCancel();
    bool isNerveEnd();
    void tryEnd();
    bool isEnableInput();
    void tryUp();
    void tryDown();
    void tryDecide();
    void tryDecideWithoutEnd();
    void tryCancel();
    void setCursorToPane();
    void tryCancelWithoutEnd();
    void exeHide();
    void exeAppear();
    void exeWait();
    void exeDecide();
    void exeDecideAfter();
    void exeEnd();

    s32 getField134() { return field_134; }

private:
    s32 field_12C = 0;
    s32 field_130 = -1;
    s32 field_134 = -1;
    s32 field_138 = -1;
    bool field_13C = false;
    s32 field_140 = -1;
    sead::PtrArray<al::LayoutActor> mParListArray;
    al::LayoutActor* mCursorActor;
    al::LayoutActor* mButtonActor;
};
}  // namespace al
