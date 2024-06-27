#pragma once

#include <heap/seadHeap.h>
#include "Library/Thread/FunctorV0M.h"

namespace agl {
class DrawContext;
}

namespace al {
class CameraDirector;
class GraphicsSystemInfo;
class ExecuteDirector;

class EffectSystem {
public:
    static EffectSystem* initializeSystem(agl::DrawContext*, sead::Heap*);

    EffectSystem();

    void initScene();
    void startScene(ExecuteDirector*);
    void endScene();
    void endInit();
    void setCameraDirector(CameraDirector*);
    void setGraphicsSystemInfo(const GraphicsSystemInfo*);

    void setField69(bool field) { field_69 = field; };

private:
    char unknown[0x69];  // TODO: incomplete
    bool field_69;
};

}  // namespace al
