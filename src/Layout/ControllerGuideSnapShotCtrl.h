#pragma once

#include <basis/seadTypes.h>

namespace al {
class LayoutInitInfo;
class PostProcessingFilter;
}  // namespace al

class ControllerGuideSnapShotCtrl {
public:
    ControllerGuideSnapShotCtrl(const char*, const al::LayoutInitInfo&,
                                al::PostProcessingFilter*);

private:
    u8 _padding[0x68];
};

static_assert(sizeof(ControllerGuideSnapShotCtrl) == 0x68);
