#pragma once

#include <container/seadPtrArray.h>

#include "Library/Scene/ISceneObj.h"

#include "Scene/SceneObjFactory.h"

namespace al {
class AreaObjGroup;
class HitSensor;
}  // namespace al

class SphinxQuiz;

class SphinxQuizRouteKillExecutor : public al::ISceneObj {
public:
    static constexpr s32 sSceneObjId = SceneObjID_SphinxQuizRouteKillExecutor;

    SphinxQuizRouteKillExecutor();

    void registerKillSensor(al::HitSensor* sensor);
    void sendMsgSphinxQuizRouteKillInArea(al::HitSensor* sender, const al::AreaObjGroup* areaGroup);

    const char* getSceneObjName() const override;
    ~SphinxQuizRouteKillExecutor() override;

private:
    sead::PtrArray<al::HitSensor> mSensors;
    al::HitSensor* mSensorBuf[128];
};

static_assert(sizeof(SphinxQuizRouteKillExecutor) == 0x418);

namespace rs {
void tryCreateSphinxQuizRouteKillExecutor(SphinxQuiz* quiz);
void sendMsgSphinxQuizRouteKill(al::HitSensor* sender, const al::AreaObjGroup* areaGroup);
}  // namespace rs
