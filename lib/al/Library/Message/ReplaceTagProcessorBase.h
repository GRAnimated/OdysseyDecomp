#pragma once

#include <basis/seadTypes.h>
#include <cstdarg>

namespace al {
class IUseMessageSystem;
class MessageTag;

class ReplaceTagProcessorBase {
public:
    virtual ~ReplaceTagProcessorBase() = default;

    void replace(char16* dst, const IUseMessageSystem* sys, const char16* src) const;
    void replaceArgs(char16* dst, s32 count, const IUseMessageSystem* sys, const char16* src,
                     ...) const;

protected:
    void replaceNumberGroup(char16* dst, const MessageTag& tag, va_list args) const;
    void replaceStringGroup(char16* dst, const MessageTag& tag, va_list args) const;
    virtual void replaceTagLabel(char16* dst, const MessageTag& tag) const;
};
}  // namespace al
