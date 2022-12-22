#include "Base/String.h"
#include "prim/seadStringUtil.h"

namespace al {

    bool isEqualString(char16_t const *a, char16_t const *b) {
        u16 cur;

        while (true) {
            cur = *a;
            if (cur != *b)
                break;

            ++b;
            ++a;
            if (!cur)
                return true;
        }
        return false;
    }

    bool isEqualSubString(char const *a, char const *b) { return strstr(a, b); }
    bool isEqualSubString(sead::SafeStringBase<char> const &a, sead::SafeStringBase<char> const &b) { return strstr(a.cstr(), b.cstr()); }

    bool isStartWithString(char const *str, char const *prefix) {
        if (prefix[0] == '\0')
            return true;

        u8 prefix_char = prefix[0];
        const char *prefix_iter = prefix + 1;

        while (*str && *str == prefix_char) {
            prefix_char = *prefix_iter++;
            str++;
            if (prefix_char == '\0')
                return true;
        }

        return false;
    }

#ifdef NON_MATCHING
    bool isEndWithString(const char *str, const char *suffix) {
        int string_len = strlen(str);
        int suffix_len = strlen(suffix);

        if (string_len >= suffix_len) {
            return isEqualString(str + (string_len - suffix_len), suffix);
        } else
            return false;
    }
#endif

    bool isMatchString(char const *str, al::MatchStr const &match) {
        auto result = al::getSubStringUnmatched(str, match);
        if (result) {
            return result[0] == 0;
        }
        return false;
    }

#ifdef NON_MATCHING
    void makeUrlDecodeString(char *url, uint maxLen, const char *src) {
        *url = 0;
        memset(url, 0, maxLen);
        if (!*src)
            return;
        int numBytesToCopy;
        while (true) {
            char decodedByte = 0;
            char asciiValue1 = *(unsigned char *)src;
            if (asciiValue1 == '%') {
                char hexChar1 = *((unsigned char *)src + 1);
                char offsetHexChar1 = hexChar1 >= 'A' ? -55 : -48;
                char decodedBytePartial = offsetHexChar1 + hexChar1;
                char hexChar2 = *((unsigned char *)src + 2);
                char offsetHexChar2 = hexChar2 >= 'A' ? -55 : -48;
                decodedByte = (offsetHexChar2 + hexChar2) | (16 * decodedBytePartial);
                numBytesToCopy = 3;
            } else {
                decodedByte = asciiValue1 == '+' ? ' ' : *src;
                numBytesToCopy = 1;
            }
            if (strlen(url) + numBytesToCopy > maxLen)
                break;
            strcat(url, &decodedByte);
            src += numBytesToCopy;
            if (!*src)
                return;
        }
        url[maxLen - 1] = 0;
        return;
    }
#endif

    void copyString(char *str, char const *copy, unsigned int range) { strncpy(str, copy, range); }
    void copyStringW(char16_t *str, char16_t const *copy, unsigned int range) { sead::StringUtil::wcs16cpy(str, range, copy); }

    bool isInStack(void const *addr) { return sead::MemUtil::isStack(addr); }

    bool isEqualString(char const *a, char const *b) {
        while (*a == *b) {
            char cur = *a;

            ++b;
            ++a;
            if (!cur)    // End of string
                return true;
        }
        return false;
    }

    bool isEqualString(sead::SafeStringBase<char> const &a, sead::SafeStringBase<char> const &b) {
        auto strA = a.cstr();
        auto strB = b.cstr();
        while (*strA == *strB) {
            char cur = *strA;

            ++strB;
            ++strA;

            if (!cur)    // End of string
                return true;
        }
        return false;
    }

    // case-insensitive
    bool isEqualStringCase(char const *a, char const *b) { return !strcasecmp(a, b); }
    bool isEqualStringCase(sead::SafeStringBase<char> const &a, sead::SafeStringBase<char> const &b) { return !strcasecmp(a.cstr(), b.cstr()); }
}    // namespace al