#pragma once

#include <basis/seadTypes.h>

namespace al {
class WipeSimple;

class WipeHolder {
public:
    WipeHolder(s32);
    void registerWipe(const char*, al::WipeSimple*);
    void startClose(const char*, s32);
    void findWipe(const char*) const;
    void startCloseByInfo(const char*);
    void findInfo(const char*) const;
    void tryStartClose(const char*, s32);
    void tryStartCloseByInfo(const char*);
    void startCloseEnd(const char*);
    void startOpen(s32);
    bool isExistInfo(const char*) const;
    void tryFindInfo(const char*) const;
    void getCloseTimeByInfo(const char*) const;
    bool isCloseEnd() const;
    bool isOpenEnd() const;
    bool isCloseWipe(const char*) const;

    bool getField18() const { return field_18; }

private:
    void* filler[3];
    bool field_18;
    void* filler2[3];
};
}  // namespace al