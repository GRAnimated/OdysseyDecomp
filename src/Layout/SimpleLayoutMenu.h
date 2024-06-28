#pragma once

#include "Library/Layout/LayoutActor.h"

namespace al {
class LayoutInitInfo;
}

class SimpleLayoutMenu : public al::LayoutActor {
public:
    SimpleLayoutMenu(const char*, const char*, const al::LayoutInitInfo&, const char*, bool);
    SimpleLayoutMenu(al::LayoutActor*, const char*, const char*, const al::LayoutInitInfo&,
                     const char*);

    void startAppear(const char*);
    void startEnd(const char*);
    void exeAppear();
    void exeWait();
    void exeEnd();
    void exeEndWait();
    bool isAppearOrWait() const;
    bool isWait() const;
    bool isEndWait() const;

private:
    int field_12C = -1;
};
