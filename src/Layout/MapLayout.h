#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Layout/LayoutActor.h"
#include "Library/Scene/ISceneObj.h"

namespace al {
class IUseSceneObjHolder;
class LayoutInitInfo;
class PlayerHolder;
}  // namespace al

class MapLayout : public al::LayoutActor, public al::ISceneObj {
public:
    MapLayout(const al::LayoutInitInfo&, const al::PlayerHolder*, s32);

private:
    u8 _padding[0x160];
};

static_assert(sizeof(MapLayout) == 0x298);

namespace rs {
void calcTransOnMap(sead::Vector2f*, const sead::Vector3f&, const sead::Matrix44f&,
                    const sead::Vector2f&, f32, f32);
bool tryCalcMapNorthDir(sead::Vector3f*, const al::IUseSceneObjHolder*);
const sead::Matrix44f& getMapViewProjMtx(const al::IUseSceneObjHolder*);
const sead::Matrix44f& getMapProjMtx(const al::IUseSceneObjHolder*);
void appearMapWithHint(const al::IUseSceneObjHolder*);
void addAmiiboHintToMap(const al::IUseSceneObjHolder*);
void appearMapWithAmiiboHint(const al::IUseSceneObjHolder*);
void appearMapMoonRockDemo(const al::IUseSceneObjHolder*, s32);
void endMap(const al::IUseSceneObjHolder*);
bool isEndMap(const al::IUseSceneObjHolder*);
bool isEnableCheckpointWarp(const al::IUseSceneObjHolder*);
}  // namespace rs

namespace StageMapFunction {
f32 getStageMapScaleMin();
f32 getStageMapScaleMax();
}  // namespace StageMapFunction
