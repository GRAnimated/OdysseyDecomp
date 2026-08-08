#include "Library/Draw/GraphicsFunction.h"

#include "Library/Projection/Projection.h"

namespace alModelFunction {
bool isShaderAssignAlphaMask(const nn::g3d::MaterialObj* material);
}

namespace al {

bool isUsingReverseProjection() {
    return isProjectionReverse();
}

bool isDepthFuncReverse() {
    return isProjectionReverse();
}

f32 getDepthClearValue() {
    return isProjectionReverse() ? 0.0f : 1.0f;
}

bool getAlphaTestEnable(const nn::g3d::MaterialObj* material) {
    return alModelFunction::isShaderAssignAlphaMask(material);
}

void setBlendCtrlToContext(sead::GraphicsContextMRT* context,
                           const nn::g3d::MaterialObj* material) {}

}  // namespace al
