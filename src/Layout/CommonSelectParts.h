#pragma once

#include <prim/seadEnum.h>

#include "Library/Nerve/NerveExecutor.h"

namespace al {
class EventFlowChoiceInfo;
class LayoutActor;
class LayoutInitInfo;
}  // namespace al

class CommonSelectParts : public al::NerveExecutor {
public:
    SEAD_ENUM(SelectPartsState, Active, Deactive);

    CommonSelectParts(const char* name, al::LayoutActor* layoutActor,
                      const al::LayoutInitInfo& initInfo, s32 maxItemCount,
                      bool hasStateData = false);
    ~CommonSelectParts() override;

    void activateAll();
    void startSelect2(const char16* text0, const char16* text1, s32 defaultIdx);
    void startSelectWithChoiceTable(const char16** textTable, s32 itemCount, s32 defaultIdx);
    void startSelectWithChoiceTableWithoutPosAnim(const char16** textTable, s32 itemCount,
                                                  s32 defaultIdx);
    void startSelectWithChoiceInfo(const al::EventFlowChoiceInfo* choiceInfo);

    bool isActive() const;
    bool isDecideEnd() const;

    void setSelectPartsString(const char16* text, s32 idx);
    void deactivate(s32 idx);
    void kill();
    void reset();

    void exeHide();
    void exeAppearBefore();
    void exeAppear();
    void exeAppearAfter();
    void exeAppearCursor();
    void exeSelect();
    void exeDecideParts();
    void exeDecideDeactiveParts();
    void exeDecide();
    void exeDecideAfter();
    void exeDecideEnd();

    s32 getSelectedIdx() const { return mSelectedIdx; }
    s32 getDecidedIdx() const { return mDecidedIdx; }

private:
    al::LayoutActor* mLayoutActor = nullptr;
    s32 mItemCount = 0;
    s32 mSelectedIdx = 0;
    s32 mDecidedIdx = 0;
    s32 mMaxItemCount = 0;
    al::LayoutActor** mItemParts = nullptr;
    al::LayoutActor* mCursorActor = nullptr;
    s32 mDefaultIdx = -1;
    s32 mScrollCounter = 0;
    s32 mSelectionCooldown = -1;
    bool mIsWithoutPosAnim = false;
    bool mIsAllowPauseCancel = false;
    s32 mStateCount = 0;
    SelectPartsState* mStateArray = nullptr;
};

static_assert(sizeof(CommonSelectParts) == 0x58);
