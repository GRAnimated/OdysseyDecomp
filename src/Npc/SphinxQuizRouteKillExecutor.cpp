#include "Npc/SphinxQuizRouteKillExecutor.h"

#include "Library/Area/AreaObjUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Scene/SceneObjUtil.h"

#include "Npc/SphinxQuiz.h"
#include "Util/PlayerDemoUtil.h"
#include "Util/SensorMsgFunction.h"

SphinxQuizRouteKillExecutor::SphinxQuizRouteKillExecutor() {
    mSensors.setBuffer(128, mSensorBuf);
}

void SphinxQuizRouteKillExecutor::registerKillSensor(al::HitSensor* sensor) {
    mSensors.pushBack(sensor);
}

void SphinxQuizRouteKillExecutor::sendMsgSphinxQuizRouteKillInArea(
    al::HitSensor* sender, const al::AreaObjGroup* areaGroup) {
    s32 count = mSensors.size();
    for (s32 i = 0; i < count; i++) {
        al::HitSensor* sensor = mSensors.at(i);
        const sead::Vector3f& trans = al::getActorTrans(sensor);
        if (al::isInAreaObj(areaGroup, trans))
            rs::sendMsgSphinxQuizRouteKill(sensor, sender);
    }
}

const char* SphinxQuizRouteKillExecutor::getSceneObjName() const {
    return "クイズスフィンクスのルート整理";
}

SphinxQuizRouteKillExecutor::~SphinxQuizRouteKillExecutor() = default;

void rs::tryCreateSphinxQuizRouteKillExecutor(SphinxQuiz* quiz) {
    if (!al::isExistSceneObj(quiz, SceneObjID_SphinxQuizRouteKillExecutor))
        al::setSceneObj(quiz, new SphinxQuizRouteKillExecutor(),
                        SceneObjID_SphinxQuizRouteKillExecutor);
}

void rs::sendMsgSphinxQuizRouteKill(al::HitSensor* sender, const al::AreaObjGroup* areaGroup) {
    al::LiveActor* host = al::getSensorHost(sender);
    auto* executor = static_cast<SphinxQuizRouteKillExecutor*>(
        al::getSceneObj(host, SceneObjID_SphinxQuizRouteKillExecutor));
    executor->sendMsgSphinxQuizRouteKillInArea(sender, areaGroup);
}

void rs::tryRegisterSphinxQuizRouteKillSensorAfterPlacement(al::HitSensor* sensor) {
    if (al::isExistSceneObj(al::getSensorHost(sensor),
                            SceneObjID_SphinxQuizRouteKillExecutor)) {
        auto* executor = static_cast<SphinxQuizRouteKillExecutor*>(
            al::getSceneObj(al::getSensorHost(sensor), SceneObjID_SphinxQuizRouteKillExecutor));
        executor->registerKillSensor(sensor);
    }
}
