#pragma once

namespace al {
class AreaObjGroup;
class FluidSurfaceHolder;
class LiveActor;

class NatureDirector {
public:
    NatureDirector();
    void init();

    void setWaterAreaGroup(AreaObjGroup* waterAreaGroup) { mWaterAreaGroup = waterAreaGroup; }

private:
    AreaObjGroup* mWaterAreaGroup;
    FluidSurfaceHolder* mFluidSurfaceHolder;
};

static_assert(sizeof(NatureDirector) == 0x10);

bool isInWater(const al::LiveActor*);

}  // namespace al
