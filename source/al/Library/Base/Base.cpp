#include "Base/Base.h"
#include "basis/seadTypes.h"
#include <cstring>

namespace al {
    char const *getBaseName(char const *str) {
        auto s = strchr(str, 47);
        if (s)
            return s + 1;
        return str;
    }
}    // namespace al