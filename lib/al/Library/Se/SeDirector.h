#pragma once

namespace al {
class SeDirector {
public:
    void setMuteSeInPVList(const char** list) { mMuteSeInPVList = list; }
    void setMuteSeInPVListSize(s32 size) { mMuteSeInPVListSize = size; }

private:
    unsigned char _padding[0xa8];
    const char** mMuteSeInPVList;
    s32 mMuteSeInPVListSize;
};
}  // namespace al
