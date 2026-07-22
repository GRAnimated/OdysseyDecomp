#include "Library/LiveActor/LiveActorKeeper.h"

#include "Library/Base/StringUtil.h"
#include "Library/LiveActor/ActorInitFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorResourceFunction.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/LiveActor/LiveActorInfo.h"
#include "Library/Obj/BreakModel.h"
#include "Library/Obj/CollisionObj.h"
#include "Library/Obj/DepthShadowModel.h"
#include "Library/Obj/ModelDrawParts.h"
#include "Library/Obj/PartsFunction.h"
#include "Library/Obj/PartsModel.h"
#include "Library/Obj/SilhouetteModel.h"
#include "Library/Obj/SimpleCircleShadowXZ.h"
#include "Library/Yaml/ByamlUtil.h"

namespace al {

SubActorInfo::SubActorInfo() {
    syncType = SubActorSync::cNone;
    subActor = nullptr;
    field_8 = nullptr;
}

namespace {

static void initSubActorInfo(SubActorInfo* actorInfo, LiveActor* rootActor,
                             const ActorInitInfo& initInfo, const ByamlIter& subActorIter) {
    const char* actorObjectName = tryGetByamlKeyStringOrNULL(subActorIter, "ObjectName");
    const char* actorModelName = tryGetByamlKeyStringOrNULL(subActorIter, "ModelName");
    const char* actorSuffix = tryGetByamlKeyStringOrNULL(subActorIter, "InitFileSuffixName");
    const char* actorClassName = tryGetByamlKeyStringOrNULL(subActorIter, "ClassName");
    const char* actorCategoryName = tryGetByamlKeyStringOrNULL(subActorIter, "CategoryName");

    bool isAlive = false;
    bool isAliveResult = subActorIter.tryGetBoolByKey(&isAlive, "IsAlive");

    bool isUseHostPlacementInfo = true;
    subActorIter.tryGetBoolByKey(&isUseHostPlacementInfo, "IsUseHostPlacementInfo");

    bool isSyncAppear = false;
    bool isGotSyncAppear = tryGetByamlBool(&isSyncAppear, subActorIter, "IsSyncAppear");
    if (isSyncAppear)
        actorInfo->syncType |= SubActorSync::cAppear;

    bool isSyncHide = false;
    bool isGotSyncHide = tryGetByamlBool(&isSyncHide, subActorIter, "IsSyncHide");
    if (isSyncHide)
        actorInfo->syncType |= SubActorSync::cHide;

    if (tryGetByamlKeyBoolOrFalse(subActorIter, "IsSyncAlphaMask"))
        actorInfo->syncType |= SubActorSync::cAlphaMask;

    if (tryGetByamlKeyBoolOrFalse(subActorIter, "IsSyncClipping"))
        actorInfo->syncType |= SubActorSync::cClipping;

    bool isCalcDepthShadowLength = true;
    tryGetByamlBool(&isCalcDepthShadowLength, subActorIter, "IsCalcDepthShadowLength");

    if (!actorClassName || isEqualString(actorClassName, "LiveActor")) {
        actorInfo->subActor = new LiveActor(actorObjectName);
        if (isUseHostPlacementInfo)
            initActorWithArchiveName(actorInfo->subActor, initInfo, actorModelName, actorSuffix);
        else
            initChildActorWithArchiveNameNoPlacementInfo(actorInfo->subActor, initInfo,
                                                         actorModelName, actorSuffix);
    } else if (isEqualString(actorClassName, "PartsModel")) {
        const char* actorFixFileSuffixName =
            tryGetByamlKeyStringOrNULL(subActorIter, "FixFileSuffixName");
        PartsModel* partsModel = new PartsModel(actorObjectName);
        partsModel->initPartsFixFileNoRegister(rootActor, initInfo, actorModelName, actorSuffix,
                                               actorFixFileSuffixName);
        actorInfo->subActor = partsModel;

        if (!isGotSyncAppear)
            actorInfo->syncType |= SubActorSync::cAppear;
        actorInfo->syncType |= SubActorSync::cClipping;

        if (isExistModel(partsModel)) {
            if (!isGotSyncHide)
                actorInfo->syncType |= SubActorSync::cHide;
            actorInfo->syncType |= SubActorSync::cAlphaMask;
        }
    } else if (isEqualString(actorClassName, "BreakModel")) {
        const char* actionName = tryGetByamlKeyStringOrNULL(subActorIter, "ActionName");
        const char* jointName = tryGetByamlKeyStringOrNULL(subActorIter, "JointName");

        sead::Matrix34f* jointMtxPtr = nullptr;
        if (jointName)
            jointMtxPtr = getJointMtxPtr(rootActor, jointName);

        if (!actionName)
            actionName = "Break";

        BreakModel* breakModel = new BreakModel(rootActor, actorObjectName, actorModelName,
                                                actorSuffix, jointMtxPtr, actionName);
        initCreateActorNoPlacementInfo(breakModel, initInfo);
        actorInfo->subActor = breakModel;
    } else if (isEqualString(actorClassName, "SilhouetteModel")) {
        actorInfo->subActor = new SilhouetteModel(rootActor, initInfo, actorCategoryName);
    } else if (isEqualString(actorClassName, "DepthShadowModel")) {
        bool calcDepthShadowLength = isCalcDepthShadowLength;
        if (actorModelName) {
            actorInfo->subActor = new DepthShadowModel(rootActor, initInfo, actorModelName,
                                                       actorSuffix, calcDepthShadowLength);
        } else {
            actorInfo->subActor =
                new DepthShadowModel(rootActor, initInfo, actorCategoryName, calcDepthShadowLength);
        }
    } else if (isEqualString(actorClassName, "InvincibleModel")) {
        actorInfo->subActor =
            new ModelDrawParts("無敵モデル", rootActor, initInfo, actorCategoryName);
    } else if (isEqualString(actorClassName, "SimpleCircleShadowXZ")) {
        SimpleCircleShadowXZ* dropShadow = new SimpleCircleShadowXZ(actorObjectName);
        dropShadow->initSimpleCircleShadow(rootActor, initInfo, actorModelName, actorSuffix);
        actorInfo->subActor = dropShadow;
    } else if (isEqualString(actorClassName, "CollisionObj")) {
        const char* collSuffixName =
            tryGetByamlKeyStringOrNULL(subActorIter, "CollisionSuffixName");
        const char* collName = tryGetByamlKeyStringOrNULL(subActorIter, "CollisionName");
        const char* sensorName = tryGetByamlKeyStringOrNULL(subActorIter, "SensorName");
        const char* fileSuffixName = tryGetByamlKeyStringOrNULL(subActorIter, "InitFileSuffixName");
        const char* jointName = tryGetByamlKeyStringOrNULL(subActorIter, "JointName");

        if (!collName)
            collName = collSuffixName;
        if (!sensorName)
            sensorName = collSuffixName;
        if (fileSuffixName)
            collSuffixName = fileSuffixName;

        auto* sensor = getHitSensor(rootActor, sensorName);
        actorInfo->subActor =
            createCollisionObj(rootActor, initInfo, collName, sensor, jointName, collSuffixName);
        if (actorObjectName)
            actorInfo->subActor->setName(actorObjectName);
    } else {
        actorInfo->subActor = new LiveActor(actorObjectName);
        initActorWithArchiveName(actorInfo->subActor, initInfo, actorModelName, actorSuffix);
    }

    initActorModelForceCubeMap(actorInfo->subActor, initInfo);

    if (isAliveResult) {
        if (isAlive)
            actorInfo->subActor->makeActorAlive();
        else
            actorInfo->subActor->makeActorDead();
    }

    bool isShow = false;
    if (tryGetByamlBool(&isShow, subActorIter, "IsShow") && !isShow)
        hideModel(actorInfo->subActor);
}

}  // namespace

SubActorKeeper::SubActorKeeper(LiveActor* rootActor) {
    mRootActor = rootActor;
}

void SubActorKeeper::registerSubActor(LiveActor* subActor, u32 syncType) {
    mBuffer[mCurActorCount] = new SubActorInfo(subActor, static_cast<SubActorSync>(syncType));
    mCurActorCount++;
}

void SubActorKeeper::init(const ActorInitInfo& initInfo, const char* suffix, s32 maxSubActors) {
    StringTmp<128> actorInitFileName;

    if (isExistModelResource(mRootActor) &&
        !tryGetActorInitFileName(&actorInitFileName, mRootActor, "InitSubActor", suffix))
        createFileNameBySuffix(&actorInitFileName, "InitSubActor", suffix);

    const auto* modelResourceYaml = static_cast<const u8*>(nullptr);
    s32 creatorCount = 0;
    do {
        if (!isExistModelResource(mRootActor) ||
            !isExistModelResourceYaml(mRootActor, actorInitFileName.cstr(), nullptr))
            break;

        modelResourceYaml = getModelResourceYaml(mRootActor, actorInitFileName.cstr(), nullptr);
        ByamlIter modelResourceIter(modelResourceYaml);
        ByamlIter initInfoIter;
        if (modelResourceIter.tryGetIterByKey(&initInfoIter, "InitInfo")) {
            s32 addActorNum = 0;
            if (initInfoIter.tryGetIntByKey(&addActorNum, "AddActorNum"))
                maxSubActors += addActorNum;
        }

        ByamlIter creatorIter;
        if (!modelResourceIter.tryGetIterByKey(&creatorIter, "CreatorList"))
            break;
        creatorCount = creatorIter.getSize();
    } while (false);

    mMaxActorCount = maxSubActors + creatorCount;
    mBuffer = new SubActorInfo*[mMaxActorCount];

    for (s32 i = 0; i < mMaxActorCount; ++i)
        mBuffer[i] = nullptr;

    if (modelResourceYaml && creatorCount > 0) {
        ByamlIter modelResourceIter(modelResourceYaml);
        ByamlIter creatorIter;
        modelResourceIter.tryGetIterByKey(&creatorIter, "CreatorList");

        for (s32 i = 0; i < creatorCount; ++i) {
            ByamlIter subActorIter;
            creatorIter.tryGetIterByIndex(&subActorIter, i);

            SubActorInfo* actorInfo = new SubActorInfo();
            mBuffer[i] = actorInfo;
            initSubActorInfo(actorInfo, mRootActor, initInfo, subActorIter);
            mCurActorCount++;
        }
    }
}

SubActorKeeper* SubActorKeeper::create(LiveActor* rootActor) {
    return new SubActorKeeper(rootActor);
}

SubActorKeeper* SubActorKeeper::tryCreate(LiveActor* rootActor, const char* suffix,
                                          s32 maxSubActors) {
    StringTmp<128> actorInitFileName;

    if (!isExistModelResource(rootActor) ||
        !tryGetActorInitFileName(&actorInitFileName, rootActor, "InitSubActor", suffix))
        return nullptr;

    if (maxSubActors <= 0) {
        if (!isExistModelResource(rootActor))
            return nullptr;

        if (!isExistModelResourceYaml(rootActor, actorInitFileName.cstr(), nullptr))
            return nullptr;
    }

    return new SubActorKeeper(rootActor);
}
}  // namespace al
