#pragma once

#include "cstdarg"

namespace al {
    char const *calcHashCode(char const *);
    char const*calcHashCodeLower(char const *);
    char const*calcHashCodeFmt(char const *, std::va_list);
    char const*calcHashCodeFmt(char const *, ...);
    char const*getBaseName(char const *);
}    // namespace al