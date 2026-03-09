#pragma once

#include <basis/seadTypes.h>

namespace al {

class EventFlowChoiceInfo {
public:
    EventFlowChoiceInfo(s32 maxChoices);

    void registerChoiceMessage(const char16* message);

    s32 mSelectedChoice = -1;
    s32 mCancelIndex = -1;
    s32 mMessageNum = 0;
    s32 mMaxChoices;
    const char16** mMessages = nullptr;
    const char16* mTalkMessage = nullptr;
    bool _20 = false;
};

}  // namespace al
