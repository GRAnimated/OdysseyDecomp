#pragma once

namespace al {
class LayoutInitInfo;
class PostProcessingFilter;
}  // namespace al

class ControllerGuideSnapShotCtrl {
public:
    ControllerGuideSnapShotCtrl(const char*, const al::LayoutInitInfo&,
                                al::PostProcessingFilter*);
};
