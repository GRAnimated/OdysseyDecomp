#pragma once

#include "nn/ui2d/Layout.h"

namespace al {

class LayoutPaneGroup {
public:
    LayoutPaneGroup(const char*);
    void startAnim(const char*);
    void getAnimator(const char*) const;
    void setAnimFrame(f32);
    void setAnimFrameRate(f32);
    f32 getAnimFrame() const;
    f32 getAnimFrameMax() const;
    f32 getAnimFrameMax(const char*) const;
    f32 getAnimFrameRate() const;
    bool isAnimExist(const char*) const;
    void tryGetAnimator(const char*) const;
    bool isAnimEnd() const;
    bool isAnimOneTime() const;
    bool isAnimOneTime(const char*) const;
    bool isAnimPlaying() const;
    const char* getPlayingAnimName() const;
    void pushAnimName(const char*);
    void createAnimator(nn::ui2d::Layout*);
    void animate(bool);
};

}  // namespace al