#pragma once

#include "Library/Base/StringUtil.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/LiveActor.h"

#include "Player/PlayerCostumeInfo.h"
#include "Player/PlayerFunction.h"
#include "Player/PlayerModelHolder.h"

namespace {

struct InitPlayerModelHolder {
    PlayerModelHolder* modelHolder;
    const al::ActorInitInfo& actorInitInfo;
    PlayerCostumeInfo* costume;
    const char* modelName;

    void operator()() const {
        al::StringTmp<128> modelSuffix("");
        al::tryReplaceString(&modelSuffix, modelName, "Mario", "");
        const char* modelSuffixCstr = modelSuffix.cstr();
        modelHolder->setModelSuffix(modelSuffixCstr);

        al::StringTmp<64> model2DName;
        if (costume->isEnableCostume2D())
            model2DName.format("%s2D", modelName);
        else
            model2DName.format("Mario2D");
        al::LiveActor* model2D = new al::LiveActor("ドットマリオモデル");
        PlayerFunction::initMarioModelActor2D(model2D, actorInitInfo, model2DName.cstr(),
                                              PlayerFunction::isInvisibleCap(costume));
        modelHolder->registerModel(model2D, "Normal2D");

        al::LiveActor* mini2D = new al::LiveActor("ドット死亡モデル");
        al::initChildActorWithArchiveNameNoPlacementInfo(mini2D, actorInitInfo, "Mario2DMini",
                                                         nullptr);
        mini2D->makeActorDead();
        modelHolder->registerModel(mini2D, "Mini2D");
        modelHolder->changeModel("Normal");
    }
};

}  // namespace
