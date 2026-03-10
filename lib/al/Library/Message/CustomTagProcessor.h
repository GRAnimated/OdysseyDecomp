#pragma once

#include <basis/seadTypes.h>

namespace eui {
class MessageMgr;
class FontMgr;
}  // namespace eui

namespace al {
class IUseMessageSystem;
class MessageSystem;

class CustomTagProcessor {
public:
    CustomTagProcessor(eui::MessageMgr* msgMgr, eui::FontMgr* fontMgr,
                       const MessageSystem* msgSystem);
    ~CustomTagProcessor();

    static u32 getPadStyleType(s32 playerPort);
    static u32 getPadPairType();
    static const char16* getPadStyleMessage(const IUseMessageSystem* msgSys, const char16* tagData,
                                            s32 type);
    static const char16* getPadPairMessage(const IUseMessageSystem* msgSys, const char16* tagData,
                                           s32 type);
};

}  // namespace al
