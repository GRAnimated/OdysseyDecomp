#include "Layout/CommonSelectParts.h"

#include <math/seadMathCalcCommon.h>
#include <prim/seadSafeString.h>

#include "Library/Base/StringUtil.h"
#include "Library/Controller/PadRumbleFunction.h"
#include "Library/Layout/LayoutActionFunction.h"
#include "Library/Layout/LayoutActor.h"
#include "Library/Layout/LayoutActorUtil.h"
#include "Library/Layout/LayoutInitInfo.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Util/StageInputFunction.h"

namespace {
NERVE_IMPL(CommonSelectParts, AppearBefore);
NERVE_IMPL(CommonSelectParts, Appear);
NERVE_IMPL(CommonSelectParts, AppearAfter);
NERVE_IMPL(CommonSelectParts, AppearCursor);
NERVE_IMPL(CommonSelectParts, Select);
NERVE_IMPL(CommonSelectParts, Hide);
NERVE_IMPL_(CommonSelectParts, SelectNoAppearAction, Select);
NERVE_IMPL(CommonSelectParts, DecideEnd);
NERVE_IMPL(CommonSelectParts, DecideParts);
NERVE_IMPL(CommonSelectParts, DecideDeactiveParts);
NERVE_IMPL(CommonSelectParts, DecideAfter);
NERVE_IMPL(CommonSelectParts, Decide);

NERVES_MAKE_NOSTRUCT(CommonSelectParts, AppearBefore, Appear, AppearAfter, AppearCursor, Select);
NERVES_MAKE_STRUCT(CommonSelectParts, Hide, SelectNoAppearAction, DecideEnd, DecideParts,
                   DecideDeactiveParts, DecideAfter, Decide);
}  // namespace

CommonSelectParts::CommonSelectParts(const char* name, al::LayoutActor* layoutActor,
                                     const al::LayoutInitInfo& initInfo, s32 maxItemCount,
                                     bool hasStateData)
    : al::NerveExecutor(name) {
    mLayoutActor = layoutActor;
    mItemCount = 0;
    mSelectedIdx = 0;
    mMaxItemCount = maxItemCount;

    mItemParts = new al::LayoutActor*[maxItemCount];

    for (s32 i = 0; i < maxItemCount; i++) {
        al::LayoutActor* actor = new al::LayoutActor("選択肢パーツ");
        mItemParts[i] = actor;
        al::StringTmp<32> partsName("%s%02d", "ParList", i);
        al::initLayoutPartsActor(actor, layoutActor, initInfo, partsName.cstr(), nullptr);
    }

    al::LayoutActor* cursorActor = new al::LayoutActor("カーソルパーツ");
    mCursorActor = cursorActor;
    al::initLayoutPartsActor(cursorActor, layoutActor, initInfo, "ParCursor", nullptr);

    al::startAction(mCursorActor, "Hide");

    initNerve(&NrvCommonSelectParts.Hide, 0);

    if (hasStateData) {
        s32 count = maxItemCount;
        if (count >= 1) {
            mStateArray = new (nullptr, 8, std::nothrow) SelectPartsState[count];
            if (mStateArray) {
                for (s32 i = 0; i < count; i++)
                    mStateArray[i] = SelectPartsState::Active;
                mStateCount = count;
            }
        }
        activateAll();
    }
}

CommonSelectParts::~CommonSelectParts() = default;

// NON_MATCHING: register allocation differs
void CommonSelectParts::activateAll() {
    for (s32 i = 0; i < mStateCount; i++)
        mStateArray[i] = SelectPartsState::Active;
}

void CommonSelectParts::startSelect2(const char16* text0, const char16* text1, s32 defaultIdx) {
    mItemCount = 2;
    mSelectedIdx = 0;
    mWithoutPosAnim = false;
    mDefaultIdx = defaultIdx;

    al::setPaneString(mItemParts[0], "TxtContent", text0);
    al::setPaneString(mItemParts[1], "TxtContent", text1);

    al::setNerve(this, &AppearBefore);
}

void CommonSelectParts::startSelectWithChoiceTable(const char16** textTable, s32 itemCount,
                                                   s32 defaultIdx) {
    mWithoutPosAnim = false;
    mItemCount = itemCount;
    mSelectedIdx = 0;
    mDefaultIdx = defaultIdx;

    for (s32 i = 0; i < itemCount; i++)
        al::setPaneString(mItemParts[i], "TxtContent", textTable[i]);

    al::setNerve(this, &AppearBefore);
}

// NON_MATCHING: register allocation differs
void CommonSelectParts::startSelectWithChoiceTableWithoutPosAnim(const char16** textTable,
                                                                 s32 itemCount, s32 defaultIdx) {
    mWithoutPosAnim = true;
    mItemCount = itemCount;
    mSelectedIdx = 0;
    mDefaultIdx = defaultIdx;

    for (s32 i = 0; i < itemCount; i++) {
        al::setPaneString(mItemParts[i], "TxtContent", textTable[i]);
        al::startAction(mItemParts[i], i == 0 ? "Select" : "Wait");
    }

    for (s32 i = 0; i < mStateCount; i++) {
        const char* stateName =
            SelectPartsState::text((u32)mStateCount <= (u32)i ? mStateArray[0] : mStateArray[i]);
        al::startAction(mItemParts[i], stateName, "State");
    }

    sead::Vector2f cursorPos;
    al::calcPaneTrans(&cursorPos, mItemParts[mSelectedIdx], "Cursor");
    al::setLocalTrans(mCursorActor, cursorPos);

    al::setNerve(this, &NrvCommonSelectParts.SelectNoAppearAction);
}

// NON_MATCHING: register allocation differs
void CommonSelectParts::startSelectWithChoiceInfo(const al::EventFlowChoiceInfo* choiceInfo) {
    mWithoutPosAnim = false;
    mItemCount = *(const s32*)((const char*)choiceInfo + 8);

    if (!*((const u8*)choiceInfo + 32) || mDecidedIdx < 0 || mDecidedIdx >= mItemCount)
        mSelectedIdx = 0;
    else
        mSelectedIdx = mDecidedIdx;

    s32 defaultIdx = *(const s32*)((const char*)choiceInfo + 4);
    if (defaultIdx < 0)
        defaultIdx = -1;
    mDefaultIdx = defaultIdx;

    for (s32 i = 0; i < mItemCount; i++)
        al::setPaneString(mItemParts[i], "TxtContent",
                          (*(const char16***)((const char*)choiceInfo + 16))[i]);

    al::setNerve(this, &AppearBefore);
}

bool CommonSelectParts::isActive() const {
    if (al::isNerve(this, &NrvCommonSelectParts.Hide))
        return false;
    return !al::isNerve(this, &NrvCommonSelectParts.DecideEnd);
}

bool CommonSelectParts::isDecideEnd() const {
    return al::isNerve(this, &NrvCommonSelectParts.DecideEnd);
}

void CommonSelectParts::setSelectPartsString(const char16* text, s32 idx) {
    al::setPaneString(mItemParts[idx], "TxtContent", text);
}

void CommonSelectParts::deactivate(s32 idx) {
    SelectPartsState* entry;
    if ((u32)mStateCount <= (u32)idx)
        entry = mStateArray;
    else
        entry = &mStateArray[idx];
    *entry = SelectPartsState::Deactive;
}

void CommonSelectParts::kill() {
    al::setNerve(this, &NrvCommonSelectParts.Hide);
}

void CommonSelectParts::reset() {
    mItemCount = 0;
    mSelectedIdx = -1;
    mDefaultIdx = -1;

    for (s32 i = 0; i < mMaxItemCount; i++)
        al::setPaneString(mItemParts[i], "TxtContent", reinterpret_cast<const char16*>(u""));
}

void CommonSelectParts::exeHide() {}

// NON_MATCHING: register allocation differs
void CommonSelectParts::exeAppearBefore() {
    if (!al::isGreaterEqualStep(this, 10))
        return;

    al::startAction(mLayoutActor, al::StringTmp<32>("%s%d", "Select", mItemCount).cstr(), "Select");
    if (!mWithoutPosAnim)
        al::startAction(mLayoutActor, "SelectAppear", "Pos");

    sead::Vector2f cursorPos;
    al::calcPaneTrans(&cursorPos, mItemParts[mSelectedIdx], "Cursor");
    al::setLocalTrans(mCursorActor, cursorPos);

    for (s32 i = 0; i < mItemCount; i++)
        al::startAction(mItemParts[i], i == mSelectedIdx ? "Select" : "Wait");

    for (s32 i = 0; i < mStateCount; i++) {
        const char* stateName =
            SelectPartsState::text((u32)mStateCount <= (u32)i ? mStateArray[0] : mStateArray[i]);
        al::startAction(mItemParts[i], stateName, "State");
    }

    al::setNerve(this, &Appear);
}

// NON_MATCHING: register allocation differs
void CommonSelectParts::exeAppear() {
    al::LayoutActor* cursorActor = mCursorActor;
    sead::Vector2f cursorPos = {};
    al::calcPaneTrans(&cursorPos, mItemParts[mSelectedIdx], "Cursor");
    al::setLocalTrans(cursorActor, cursorPos);

    if (al::isActionEnd(mLayoutActor, "Pos"))
        al::setNerve(this, &AppearAfter);
}

// NON_MATCHING: register allocation differs
void CommonSelectParts::exeAppearAfter() {
    al::LayoutActor* cursorActor = mCursorActor;
    sead::Vector2f cursorPos = {};
    al::calcPaneTrans(&cursorPos, mItemParts[mSelectedIdx], "Cursor");
    al::setLocalTrans(cursorActor, cursorPos);

    al::setNerveAtGreaterEqualStep(this, &AppearCursor, 5);
}

// NON_MATCHING: register allocation differs
void CommonSelectParts::exeAppearCursor() {
    if (al::isFirstStep(this))
        al::startAction(mCursorActor, "Appear");

    al::LayoutActor* cursorActor = mCursorActor;
    sead::Vector2f cursorPos = {};
    al::calcPaneTrans(&cursorPos, mItemParts[mSelectedIdx], "Cursor");
    al::setLocalTrans(cursorActor, cursorPos);

    if (al::isActionEnd(cursorActor))
        al::setNerve(this, &Select);
}

// NON_MATCHING: register allocation differs
void CommonSelectParts::exeSelect() {
    if (al::isFirstStep(this)) {
        if (!mWithoutPosAnim)
            al::startAction(mLayoutActor, "SelectWait", "Pos");
        mScrollCounter = 0;
        mSelectedIdx = -1;
        bool isNoAppearAnim = al::isNerve(this, &NrvCommonSelectParts.SelectNoAppearAction);
        al::startAction(mCursorActor, isNoAppearAnim ? "Hide" : "Wait");
    }

    if (al::isNerve(this, &NrvCommonSelectParts.SelectNoAppearAction) && al::isStep(this, 2))
        al::startAction(mCursorActor, "Wait");

    if (rs::isRepeatUiUp(mLayoutActor)) {
        s32 counter = mScrollCounter - 1;
        if (counter < 0)
            counter = 0;
        mScrollCounter = counter;
    } else if (rs::isRepeatUiDown(mLayoutActor)) {
        s32 counter = mScrollCounter;
        if (counter >= 0)
            counter++;
        else
            counter = 0;
        mScrollCounter = counter;
    } else {
        mScrollCounter = 0;
    }

    if (mSelectionCooldown < 0) {
        if (rs::isTriggerUiUp(mLayoutActor) || rs::isTriggerUiDown(mLayoutActor) ||
            sead::Mathi::abs(mScrollCounter) >= 30) {
            s32 dir;
            if (rs::isTriggerUiUp(mLayoutActor))
                dir = -1;
            else
                dir = (mScrollCounter >> 31) | 1;

            al::startAction(mItemParts[mSelectedIdx], "Wait");

            s32 newIdx = al::modi(mSelectedIdx + dir + mItemCount, mItemCount);
            mSelectedIdx = newIdx;
            al::startAction(mItemParts[newIdx], "Select");

            mSelectionCooldown = 6;

            al::PadRumbleParam rumble;
            rumble.setVolumeByBalance(0.3f);
            rumble.isUseController = true;
            alPadRumbleFunction::startPadRumbleNo3DWithParam(
                alPadRumbleFunction::getPadRumbleDirector(mLayoutActor), "UI", rumble);

            if (sead::Mathi::abs(mScrollCounter) >= 30)
                mScrollCounter -= 5 * al::sign(mScrollCounter);
        }
    }

    if (mSelectionCooldown >= 0)
        mSelectionCooldown--;

    if (mDefaultIdx >= 0) {
        bool doCancel = rs::isTriggerUiCancel(mLayoutActor);
        bool doPause = mAllowPauseCancel && rs::isTriggerUiPause(mLayoutActor);
        if (doCancel || doPause) {
            al::PadRumbleParam rumble;
            rumble.setVolumeByBalance(0.7f);
            rumble.isUseController = true;
            alPadRumbleFunction::startPadRumbleNo3DWithParam(
                alPadRumbleFunction::getPadRumbleDirector(mLayoutActor), "UI", rumble);

            al::startAction(mItemParts[mSelectedIdx], "Wait");
            mSelectedIdx = mDefaultIdx;
            al::startAction(mItemParts[mSelectedIdx], "Select");

            sead::Vector2f cursorPos;
            al::calcPaneTrans(&cursorPos, mItemParts[mSelectedIdx], "Cursor");
            al::setLocalTrans(mCursorActor, cursorPos);

            al::setNerve(this, &NrvCommonSelectParts.Decide);
            return;
        }
    }

    sead::Vector2f cursorPos;
    al::calcPaneTrans(&cursorPos, mItemParts[mSelectedIdx], "Cursor");
    al::setLocalTrans(mCursorActor, cursorPos);

    if (rs::isTriggerUiDecide(mLayoutActor)) {
        al::PadRumbleParam rumble;
        rumble.setVolumeByBalance(0.7f);
        rumble.isUseController = true;
        alPadRumbleFunction::startPadRumbleNo3DWithParam(
            alPadRumbleFunction::getPadRumbleDirector(mLayoutActor), "UI", rumble);

        if ((u32)mStateCount > (u32)mSelectedIdx &&
            mStateArray[mSelectedIdx] == SelectPartsState::Deactive) {
            al::setNerve(this, &NrvCommonSelectParts.DecideDeactiveParts);
        } else {
            al::setNerve(this, &NrvCommonSelectParts.DecideParts);
        }
    }
}

// NON_MATCHING: register allocation differs
void CommonSelectParts::exeDecideParts() {
    if (al::isFirstStep(this)) {
        al::startAction(mItemParts[mSelectedIdx], "Decide");
        al::startAction(mCursorActor, "End");
        mDecidedIdx = mSelectedIdx;
    }

    if (al::isActionEnd(mItemParts[mSelectedIdx]) && al::isActionEnd(mCursorActor)) {
        al::startAction(mCursorActor, "Hide");
        if (mWithoutPosAnim)
            al::setNerve(this, &NrvCommonSelectParts.DecideAfter);
        else
            al::setNerve(this, &NrvCommonSelectParts.Decide);
    }
}

void CommonSelectParts::exeDecideDeactiveParts() {
    if (al::isFirstStep(this)) {
        al::startAction(mItemParts[mSelectedIdx], "Decide");
        mDecidedIdx = mSelectedIdx;
    }
    al::setNerve(this, &AppearCursor);
}

// NON_MATCHING: register allocation differs
void CommonSelectParts::exeDecide() {
    if (al::isFirstStep(this))
        al::startAction(mLayoutActor, "SelectEnd", "Pos");

    if (al::isActionEnd(mLayoutActor, "Pos"))
        al::setNerve(this, &NrvCommonSelectParts.DecideAfter);
}

void CommonSelectParts::exeDecideAfter() {
    al::setNerveAtGreaterEqualStep(this, &NrvCommonSelectParts.DecideEnd, 10);
}

void CommonSelectParts::exeDecideEnd() {}
