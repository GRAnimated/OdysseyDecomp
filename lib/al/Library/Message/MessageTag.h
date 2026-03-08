#pragma once

#include <basis/seadTypes.h>

namespace nn::font {
template <typename T>
class PrintContext;
}  // namespace nn::font

namespace al {

class MessageTag {
public:
    MessageTag(const char16* str);
    MessageTag(const nn::font::PrintContext<char16>* context);

    s32 getSkipLength() const;
    s32 getParam8(s32 idx) const;
    s32 getParam16(s32 idx) const;
    s32 getParam32(s32 idx) const;
    const void* getParamPtr(s32 idx) const;

    const char16* getData() const { return mData; }

private:
    const char16* mData;
};

}  // namespace al
