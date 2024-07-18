#include "Library/Play/Layout/WindowConfirm.h"

#include "Library/Base/StringUtil.h"
#include "Library/Layout/LayoutActionFunction.h"
#include "Library/Layout/LayoutActorUtil.h"
#include "Library/Layout/LayoutInitInfo.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
using namespace al;
NERVE_IMPL(WindowConfirm, Hide);
NERVE_IMPL(WindowConfirm, Appear);
NERVE_IMPL(WindowConfirm, End);
NERVE_IMPL(WindowConfirm, Decide);
NERVE_IMPL(WindowConfirm, Wait);
NERVE_IMPL(WindowConfirm, DecideAfter);

NERVES_MAKE_NOSTRUCT(WindowConfirm, Hide, Appear, DecideAfter);
NERVES_MAKE_STRUCT(WindowConfirm, End, Decide, Wait);

}  // namespace

namespace al {
WindowConfirm::WindowConfirm(const LayoutInitInfo& info, const char* name, const char* name2)
    : LayoutActor(name2) {
    initLayoutActor(this, info, name, nullptr);

    s32 paneChildNum = getPaneChildNum(this, "All");
    s32 i = 0;
    s32 v13 = 0;
    for (i = 0; i < paneChildNum; i++)
        v13 += isExistPane(this, StringTmp<64>("ParList%02d", i).cstr());

    mParListArray.allocBuffer(v13, nullptr, 8);

    for (i = 0; i < v13; i++) {
        LayoutActor* choiceActor = new LayoutActor(StringTmp<64>("選択肢%02d", i).cstr());

        initLayoutPartsActor(choiceActor, this, info, StringTmp<64>("ParList%02d", i).cstr(),
                             nullptr);
        mParListArray.pushBack(choiceActor);
    }

    if (isExistPane(this, "ParHardKey")) {
        mButtonActor = new LayoutActor("Ａボタン");
        initLayoutPartsActor(mButtonActor, this, info, "ParHardKey", nullptr);
    }

    mCursorActor = new LayoutActor("カーソル");
    initLayoutPartsActor(mCursorActor, this, info, "ParCursor", nullptr);

    initNerve(&Hide, 0);
}

void WindowConfirm::setTxtMessage(const char16* message) {
    setPaneString(this, "TxtMessage", message, 0);
}

void WindowConfirm::setTxtList(s32 index, const char16* message) {
    setPaneString(mParListArray.at(index), "TxtContent", message, 0);
}

void WindowConfirm::setListNum(s32 num) {
    field_130 = num;
    if (num == 2)
        field_138 = 1;
    if (num == 3)
        field_138 = 0;
}

void WindowConfirm::setCancelIdx(s32 index) {
    field_138 = index;
}

void WindowConfirm::appear() {
    if (isAlive())
        return;
    field_134 = 0;
    field_12C = 0;

    startAction(this, "Appear", nullptr);
    switch (field_130) {
    case 0:
        startAction(this, "SelectHardKey", "Select");

        hidePane(this, "ParCursor");
        hidePane(this, "ParHardKey");
        break;
    case 1:
        startAction(this, "SelectHardKey", "Select");
        startAction(mButtonActor, "Appear", nullptr);

        hidePane(this, "ParCursor");
        showPane(this, "ParHardKey");
        break;
    case 2:
        startAction(this, "Select2", "Select");
        startAction(mCursorActor, "Appear", nullptr);
        startAction(mParListArray[0], "Select", nullptr);
        startAction(mParListArray[1], "Wait", nullptr);

        showPane(this, "ParCursor");
        hidePane(this, "ParHardKey");
        break;
    case 3:
        startAction(this, "Select3", "Select");
        startAction(mCursorActor, "Appear", nullptr);
        startAction(mParListArray[0], "Select", nullptr);
        startAction(mParListArray[1], "Wait", nullptr);
        startAction(mParListArray[2], "Wait", nullptr);

        showPane(this, "ParCursor");
        hidePane(this, "ParHardKey");
        break;
    default:
        break;
    }
    LayoutActor::appear();
    setNerve(this, &Appear);
}

void WindowConfirm::appearWithChoicingCancel() {
    if (isAlive())
        return;
    appear();
    if (field_130 == 2 || field_134 == 3) {
        startAction(mParListArray[field_134], "Wait", nullptr);
        startAction(mParListArray[field_138], "Select", nullptr);
    }
    field_134 = field_138;
}

bool WindowConfirm::isNerveEnd() {
    return isNerve(this, &NrvWindowConfirm.End);
}

bool WindowConfirm::tryEnd() {
    if (isEnableInput()) {
        field_13C = 0;
        setNerve(this, &NrvWindowConfirm.End);
        return true;
    }
    return false;
}

bool WindowConfirm::isEnableInput() {
    if (field_140 <= 0 && isNerve(this, &NrvWindowConfirm.Wait) && isGreaterEqualStep(this, 10)) {
        field_140 = 10;
        return true;
    }
    return false;
}

bool WindowConfirm::tryUp() {
    if (isEnableInput()) {
        field_12C = 1;
        return true;
    }
    return false;
}

bool WindowConfirm::tryDown() {
    if (isEnableInput()) {
        field_12C = 2;
        return true;
    }
    return false;
}

bool WindowConfirm::tryDecide() {
    if (isEnableInput()) {
        field_13C = 0;
        startHitReaction(this, "決定", nullptr);
        setNerve(this, &NrvWindowConfirm.Decide);
        return true;
    }
    return false;
}

bool WindowConfirm::tryDecideWithoutEnd() {
    if (isEnableInput()) {
        field_13C = 0;
        setNerve(this, &NrvWindowConfirm.Decide);
        return true;
    }
    return false;
}

bool WindowConfirm::tryCancel() {
    if (!isEnableInput())
        return false;
    if ((field_130 & 0xFFFFFFFE) == 2 && field_134 != field_138) {
        startAction(mParListArray[field_134], "Wait", nullptr);
        startAction(mParListArray[field_138], "Select", nullptr);
        field_134 = field_138;
        setCursorToPane();
    }
    startHitReaction(this, "キャンセル", nullptr);
    field_13C = 0;
    setNerve(this, &NrvWindowConfirm.Decide);
    return true;
}

void WindowConfirm::setCursorToPane() {
    if (field_130 < 2)
        return;
    sead::Vector3f trans = {1.0f, 1.0f, 1.0f};
    calcPaneTrans(&trans, mParListArray[field_134], "Cursor");
    setPaneLocalTrans(this, "ParCursor", trans);
}

bool WindowConfirm::tryCancelWithoutEnd() {
    if (isEnableInput()) {
        if ((field_130 & 0xFFFFFFFE) == 2 && field_134 != field_138) {
            startAction(mParListArray[field_134], "Wait", nullptr);
            startAction(mParListArray[field_138], "Select", nullptr);
            field_134 = field_138;
            setCursorToPane();
        }
        field_13C = 1;
        setNerve(this, &NrvWindowConfirm.Decide);
        return true;
    }
    return false;
}

void WindowConfirm::exeHide() {}

void WindowConfirm::exeAppear() {
    if ((field_130 & 0xFFFFFFFE) == 2)
        setCursorToPane();
    if (isActionEnd(this, nullptr))
        setNerve(this, &NrvWindowConfirm.Wait);
}

void WindowConfirm::exeWait() {
    if (isFirstStep(this)) {
        startAction(this, "Wait", nullptr);
        field_140 = -1;
    }
    if ((field_130 & 0xFFFFFFFE) == 1) {
        if (isActionPlaying(mButtonActor, "Appear", nullptr) && isActionEnd(mButtonActor, nullptr))
            startAction(mCursorActor, "Wait", nullptr);
    } else {
        if (isActionPlaying(mCursorActor, "Appear", nullptr) && isActionEnd(mCursorActor, nullptr))
            startAction(mCursorActor, "Wait", nullptr);
    }

    if (field_12C == 1) {
        startAction(mParListArray[field_134], "Wait", nullptr);
        field_134--;
        if (field_134 <= 0)
            field_134 = field_130 - 1;

        startAction(mParListArray[field_134], "Select", nullptr);
        if (field_130 >= 2)
            setCursorToPane();
    }
    if (field_12C == 2) {
        startAction(mParListArray[field_134], "Wait", nullptr);
        field_134++;
        if (field_134 >= field_130)
            field_134 = 0;

        startAction(mParListArray[field_134], "Select", nullptr);
        if (field_130 >= 2)
            setCursorToPane();
    }

    field_12C = 0;
    if ((field_140 & 0x80000000) == 0)
        field_140--;
}

void WindowConfirm::exeDecide() {
    if (isFirstStep(this)) {
        if ((field_130 & 0xFFFFFFFE) == 2) {
            startAction(mParListArray[field_134], "Decide", nullptr);
            startAction(mCursorActor, "End", nullptr);
        } else if ((field_130 & 0xFFFFFFFE) == 1) {
            startAction(mButtonActor, "PageEnd", nullptr);
        } else if ((field_130 & 0xFFFFFFFE) == 0) {
            setNerve(this, &DecideAfter);
        }
    }
    if ((field_130 & 0xFFFFFFFE) == 2) {
        if (isActionEnd(mParListArray[field_134], nullptr))
            setNerve(this, &DecideAfter);
    } else if (isActionEnd(mButtonActor, nullptr))
        setNerve(this, &DecideAfter);
}

void WindowConfirm::exeDecideAfter() {
    setNerveAtGreaterEqualStep(this, &NrvWindowConfirm.End, 0);
}

void WindowConfirm::exeEnd() {
    if (field_13C)
        return;
    if (isFirstStep(this))
        startAction(this, "End", nullptr);
    if (isActionEnd(this, nullptr))
        kill();
}

}  // namespace al
