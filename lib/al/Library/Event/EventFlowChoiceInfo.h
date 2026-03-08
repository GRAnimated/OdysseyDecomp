#pragma once

#include <basis/seadTypes.h>

namespace al {

class EventFlowChoiceInfo {
public:
    s32 mSelectedChoice;
    s32 mCancelIndex;
    s32 mMessageNum;
    s32 _0c;
    const char16** mMessages;
    const char16* mTalkMessage;
};

}  // namespace al
