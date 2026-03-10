#pragma once

#include "Library/Layout/LayoutActor.h"

namespace al {
class EventFlowChoiceInfo;
class LayoutInitInfo;
class LiveActor;
class MessageTagDataHolder;
}  // namespace al

class TalkMessage : public al::LayoutActor {
public:
    TalkMessage(const char* name);

    void appear() override;
    void kill() override;
    void control() override;

    void initLayoutTalk(const al::LayoutInitInfo& info, const char* suffix);
    void initLayoutImportant(const al::LayoutInitInfo& info, const char* suffix);
    void initLayoutOver(const al::LayoutInitInfo& info, const char* suffix);
    void initLayoutForEventTalk(const al::LayoutInitInfo& info);
    void initLayoutForEventImportant(const al::LayoutInitInfo& info);
    void initLayoutWithArchiveName(const al::LayoutInitInfo& info, const char* archiveName,
                                   const char* suffix);

    void startForNpc(const al::LiveActor* actor, const char16* message, const char16* name,
                     const al::MessageTagDataHolder* tagDataHolder, bool isNormal);
    void startForSystem(const char16* message, const al::MessageTagDataHolder* tagDataHolder,
                        bool isNormal);
    void reset();
    void end();

    void startSelectWithChoiceTable(const char16** choices, s32 count, s32 cancelIndex);
    void startSelectWithChoiceInfo(const al::EventFlowChoiceInfo* choiceInfo);

    bool isWait() const;
    bool isIconWait() const;
    bool isSelectDecide() const;
    s32 getSelectedChoiceIndex() const;

    void startIconPageNext();

    void exeAppear();
    void exeAppearWithText();
    void exeTextAnim();
    void exeIconAppearDelay();
    void exeIconAppear();
    void exeIconWait();
    void exeIconWaitTriggered();
    void exeIconPageNext();
    void exeIconPageNextAndPlayNextPage();
    void exeIconPageNextAndLoadNextMessage();
    void exeIconPageEnd();
    void exeWait();
    void exeEnd();
};
