#include "Library/Play/Layout/WindowConfirm.h"

#include "Library/Base/StringUtil.h"
#include "Library/Layout/LayoutActionFunction.h"
#include "Library/Layout/LayoutActorUtil.h"
#include "Library/Layout/LayoutInitInfo.h"
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
WindowConfirm::WindowConfirm(const al::LayoutInitInfo& info, const char* name, const char* name2)
    : al::LayoutActor(name2) {
    al::initLayoutActor(this, info, name, nullptr);

    s32 paneChildNum = al::getPaneChildNum(this, "All");
    s32 i = 0;
    s32 v13 = 0;
    for (i = 0; i < paneChildNum; i++)
        v13 += al::isExistPane(this, al::StringTmp<64>("ParList%02d", i).cstr());

    mParListArray.allocBuffer(v13, nullptr, 8);

    for (i = 0; i < v13; i++) {
        LayoutActor* choiceActor = new LayoutActor(al::StringTmp<64>("選択肢%02d", i).cstr());

        al::initLayoutPartsActor(choiceActor, this, info,
                                 al::StringTmp<64>("ParList%02d", i).cstr(), nullptr);
        mParListArray.pushBack(choiceActor);
    }

    if (al::isExistPane(this, "ParHardKey")) {
        mButtonActor = new LayoutActor("Ａボタン");
        al::initLayoutPartsActor(mButtonActor, this, info, "ParHardKey", nullptr);
    }

    mCursorActor = new LayoutActor("カーソル");
    al::initLayoutPartsActor(mCursorActor, this, info, "ParCursor", nullptr);

    initNerve(&Hide, 0);
}

void WindowConfirm::setTxtMessage(const char16* message) {
    al::setPaneString(this, "TxtMessage", message, 0);
}

void WindowConfirm::setTxtList(s32 index, const char16* message) {
    al::setPaneString(mParListArray.at(index), "TxtContent", message, 0);
}

void WindowConfirm::setListNum(s32 num) {
    field_130 = num;
    if (num == 3)
        field_130 = 0;
    else if (num == 2)
        field_138 = 1;
}

void WindowConfirm::setCancelIdx(s32 index) {
    field_138 = index;
}

void WindowConfirm::appear() {
    if (isAlive())
        return;
    field_134 = 0;
    field_12C = 0;

    al::startAction(this, "Appear", nullptr);
    switch (field_130) {
    case 0:
        al::startAction(this, "SelectHardKey", "Select");

        al::hidePane(this, "ParCursor");
        al::hidePane(this, "ParHardKey");
        break;
    case 1:
        al::startAction(this, "SelectHardKey", "Select");
        al::startAction(mButtonActor, "Appear", nullptr);

        al::hidePane(this, "ParCursor");
        al::showPane(this, "ParHardKey");
        break;
    case 2:
        al::startAction(this, "Select2", "Select");
        al::startAction(mCursorActor, "Appear", nullptr);
        al::startAction(mParListArray[0], "Select", nullptr);
        al::startAction(mParListArray[1], "Wait", nullptr);

        al::showPane(this, "ParCursor");
        al::hidePane(this, "ParHardKey");
        break;
    case 3:
        al::startAction(this, "Select3", "Select");
        al::startAction(mCursorActor, "Appear", nullptr);
        al::startAction(mParListArray[0], "Select", nullptr);
        al::startAction(mParListArray[1], "Wait", nullptr);
        al::startAction(mParListArray[2], "Wait", nullptr);

        al::showPane(this, "ParCursor");
        al::hidePane(this, "ParHardKey");
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
        al::startAction(mParListArray[field_134], "Wait", nullptr);
        al::startAction(mParListArray[field_138], "Select", nullptr);
    }
    field_134 = field_138;
}

bool WindowConfirm::isNerveEnd() {
    return al::isNerve(this, &NrvWindowConfirm.End);
}

void WindowConfirm::tryEnd() {}

bool WindowConfirm::isEnableInput() {}

void WindowConfirm::tryUp() {}

void WindowConfirm::tryDown() {}

void WindowConfirm::tryDecide() {}

void WindowConfirm::tryDecideWithoutEnd() {}

void WindowConfirm::tryCancel() {}

void WindowConfirm::setCursorToPane() {}

void WindowConfirm::tryCancelWithoutEnd() {}

void WindowConfirm::exeHide() {}

void WindowConfirm::exeAppear() {
    if (field_130 == 2 || field_130 == 3) {
        if (field_130 >= 2) {
            sead::Vector3f paneTrans = {1.0f, 1.0f, 1.0f};
            al::calcPaneTrans(&paneTrans, mParListArray[field_130], "Cursor");
            al::setPaneLocalTrans(this, "Cursor", paneTrans);
        }
    }
    if (al::isActionEnd(this, nullptr))
        al::setNerve(this, &NrvWindowConfirm.Wait);
}

void WindowConfirm::exeWait() {}

void WindowConfirm::exeDecide() {}

void WindowConfirm::exeDecideAfter() {
    al::setNerveAtGreaterEqualStep(this, &NrvWindowConfirm.End, 0);
}

void WindowConfirm::exeEnd() {
    if (field_13C)
        return;
    if (al::isFirstStep(this))
        al::startAction(this, "End", nullptr);
    if (al::isActionEnd(this, nullptr))
        kill();
}

}  // namespace al