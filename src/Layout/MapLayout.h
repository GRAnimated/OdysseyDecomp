#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class IUseSceneObjHolder;
}
class MapLayout;

namespace rs {
void calcTransOnMap(sead::Vector2f*, const sead::Vector3f&, const sead::Matrix44f&,
                    const sead::Vector2f&, f32, f32);
void tryCalcMapNorthDir(sead::Vector3f*, const al::IUseSceneObjHolder*);
void getMapViewProjMtx(const al::IUseSceneObjHolder*);
void getMapProjMtx(const al::IUseSceneObjHolder*);
void appearMapWithHint(const al::IUseSceneObjHolder*);
void addAmiiboHintToMap(const al::IUseSceneObjHolder*);
void appearMapWithAmiiboHint(const al::IUseSceneObjHolder*);
void appearMapMoonRockDemo(const al::IUseSceneObjHolder*, s32);
void endMap(const al::IUseSceneObjHolder*);
bool isEndMap(const al::IUseSceneObjHolder*);
bool isEnableCheckpointWarp(const al::IUseSceneObjHolder*);
}  // namespace rs

namespace StageMapFunction {
void getStageMapScaleMin();
void getStageMapScaleMax();
}  // namespace StageMapFunction
