#pragma once

#include "LiveActor.h"

namespace al {
    void showModel(al::LiveActor *);
    void hideModel(al::LiveActor *);
    bool isHideModel(al::LiveActor const*);
}