#pragma once

#include <basis/seadTypes.h>

#include "Library/Message/MessageTag.h"

namespace al {
class IUseMessageSystem;

class ReplaceTagProcessorBase {
public:
    virtual u32 replacePictureGroup(char16* out, const MessageTag& tag) const;
    virtual u32 replaceNumberGroup(char16* out, const MessageTag& tag, ...) const;
    virtual u32 replaceStringGroup(char16* out, const MessageTag& tag, ...) const;
    virtual u32 replaceTagLabel(char16* out, const MessageTag& tag) const;
    virtual u32 replaceProjectTag(char16* out, const MessageTag& tag,
                                  const IUseMessageSystem* msgSys) const;
};

}  // namespace al
