#include "Library/Event/EventFlowChoiceInfo.h"

namespace al {

EventFlowChoiceInfo::EventFlowChoiceInfo(s32 maxChoices) : mMaxChoices(maxChoices) {
    mMessages = new const char16*[maxChoices];
    for (s32 i = 0; i < mMaxChoices; i++)
        mMessages[i] = nullptr;
}

void EventFlowChoiceInfo::registerChoiceMessage(const char16* message) {
    mMessages[mMessageNum] = message;
    mMessageNum++;
}

}  // namespace al
