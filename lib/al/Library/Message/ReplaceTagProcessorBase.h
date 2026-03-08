#pragma once

#include <basis/seadTypes.h>

namespace al {
class IUseMessageSystem;

class ReplaceTagProcessorBase {
public:
    virtual ~ReplaceTagProcessorBase() = default;

    void replace(char16* dst, const IUseMessageSystem* sys, const char16* src) const;
};
}  // namespace al
