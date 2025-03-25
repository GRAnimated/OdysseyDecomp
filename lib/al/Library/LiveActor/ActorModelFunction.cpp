#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/Area/AreaObjUtil.h"
#include "Library/Base/StringUtil.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/Draw/GraphicsSystemInfo.h"
#include "Library/Effect/EffectKeeper.h"
#include "Library/Effect/EffectSystemInfo.h"
#include "Library/Execute/ExecuteUtil.h"
#include "Library/Light/MaterialCategoryKeeper.h"
#include "Library/Light/ModelMaterialCategory.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorCollisionFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSceneInfo.h"
#include "Library/LiveActor/LiveActorFlag.h"
#include "Library/LiveActor/SubActorKeeper.h"
#include "Library/Math/MathUtil.h"
#include "Library/Matrix/MatrixUtil.h"
#include "Library/Model/ModelCtrl.h"
#include "Library/Model/ModelFunction.h"
#include "Library/Model/ModelKeeper.h"
#include "Library/Model/ModelShapeUtil.h"
#include "Library/Nature/NatureUtil.h"
#include "Library/Obj/ActorDitherAnimator.h"
#include "Library/Se/SeFunction.h"
#include "Library/Shader/ForwardRendering/EnvTextureKeeper.h"
#include "Library/Shadow/ActorShadowUtil.h"
#include "Project/Light/ActorPrepassLightKeeper.h"
#include "math/seadMatrix.h"
#include "math/seadMatrixCalcCommon.h"
#include "math/seadVectorFwd.h"
#include "nn/g3d/ModelObj.h"

namespace al {
void updateMaterialCodeGround(LiveActor* actor, const char* material) {
    if (!isCollidedGround(actor))
        setMaterialCode(actor, material);
    else {
        const char* floorMaterial = getCollidedFloorMaterialCodeName(actor);
        if (floorMaterial)
            setMaterialCode(actor, floorMaterial);
    }
}

void updateMaterialCodeAll(LiveActor* actor) {
    if (isCollidedGround(actor)) {
        const char* floorMaterial = getCollidedFloorMaterialCodeName(actor);
        if (floorMaterial) {
            setMaterialCode(actor, floorMaterial);
            return;
        }
    }

    if (isCollidedWall(actor)) {
        const char* wallMaterial = getCollidedWallMaterialCodeName(actor);
        if (wallMaterial) {
            setMaterialCode(actor, wallMaterial);
            return;
        }
    }

    if (isCollidedCeiling(actor)) {
        const char* ceilingMaterial = getCollidedCeilingMaterialCodeName(actor);
        if (ceilingMaterial)
            setMaterialCode(actor, ceilingMaterial);
    }
}

void updateMaterialCodeArea(LiveActor* actor) {
    AreaObj* area = tryFindAreaObj(actor, "MaterialCodeArea", getTrans(actor));
    if (area) {
        const char* materialCode;
        if (tryGetAreaObjStringArg(&materialCode, area, "MaterialCodeType")) {
            if (isEqualString(materialCode, "Wet"))
                updateMaterialCodeWet(actor, true);
        } else {
            updateMaterialCodeWet(actor, false);
        }
    } else {
        updateMaterialCodeWet(actor, false);
    }
}

void updateMaterialCodeWet(LiveActor* actor, bool isWet) {
    if (actor->getEffectKeeper())
        updateEffectMaterialWet(actor, isWet);
    if (isExistSeKeeper(actor))
        updateSeMaterialWet(actor, isWet);
}

void updateMaterialCodeWater(LiveActor* actor) {
    updateMaterialCodeWater(actor, isInWater(actor));
}

void updateMaterialCodeWater(LiveActor* actor, bool isInWater) {
    if (actor->getEffectKeeper())
        updateEffectMaterialWater(actor, isInWater);
    if (isExistSeKeeper(actor))
        updateSeMaterialWater(actor, isInWater);
}

void updateMaterialCodePuddle(LiveActor* actor) {
    const sead::Vector3f& gravity = -al::getGravity(actor);
    sead::Vector3f unk = {0.0f, 0.0f, 0.0f};
    sead::Vector3f unk2 = {0.0f, 0.0f, 0.0f};
    const sead::Vector3f& trans = al::getTrans(actor);

    bool isWet = false;
    if (al::calcFindWaterSurface(&unk, &unk2, actor, trans, gravity, 80.0f)) {
        const sead::Vector3f& trans2 = al::getTrans(actor);
        isWet = (unk - trans2).dot(gravity) > 0.0f;
    }
    updateMaterialCodePuddle(actor, isWet);
}

void updateMaterialCodePuddle(LiveActor* actor, bool isWet) {
    if (actor->getEffectKeeper())
        updateEffectMaterialPuddle(actor, isWet);
    if (isExistSeKeeper(actor))
        updateSeMaterialPuddle(actor, isWet);
}

void resetMaterialCode(LiveActor* actor) {
    if (actor->getEffectKeeper())
        resetEffectMaterialCode(actor);
    if (isExistSeKeeper(actor))
        resetSeMaterialName(actor);
}

void showModel(LiveActor* actor) {
    if (isDead(actor) || isClipped(actor)) {
        actor->getFlags()->isModelHidden = false;
        return;
    }

    ModelKeeper* modelKeeper = actor->getModelKeeper();
    if (modelKeeper)
        modelKeeper->show();

    if (actor->getShadowKeeper())
        al::showShadow(actor);

    alActorSystemFunction::addToExecutorDraw(actor);

    SubActorKeeper* subActorKeeper = actor->getSubActorKeeper();
    if (subActorKeeper)
        alSubActorFunction::trySyncShowModel(subActorKeeper);

    ActorPrePassLightKeeper* prePassLightKeeper = actor->getActorPrePassLightKeeper();
    if (prePassLightKeeper)
        prePassLightKeeper->appear(false);

    actor->getFlags()->isModelHidden = false;
}

void showModelIfHide(LiveActor* actor) {
    if (isHideModel(actor))
        showModel(actor);
}

bool isHideModel(const LiveActor* actor) {
    return actor->getFlags()->isModelHidden;
}

void hideModel(LiveActor* actor) {
    if (isDead(actor) || isClipped(actor)) {
        actor->getFlags()->isModelHidden = true;
        return;
    }

    ModelKeeper* modelKeeper = actor->getModelKeeper();
    if (modelKeeper)
        modelKeeper->hide();

    if (actor->getShadowKeeper())
        al::hideShadow(actor);

    alActorSystemFunction::removeFromExecutorDraw(actor);

    SubActorKeeper* subActorKeeper = actor->getSubActorKeeper();
    if (subActorKeeper)
        alSubActorFunction::trySyncHideModel(subActorKeeper);

    ActorPrePassLightKeeper* prePassLightKeeper = actor->getActorPrePassLightKeeper();
    if (prePassLightKeeper)
        prePassLightKeeper->hideModel();

    actor->getFlags()->isModelHidden = true;
}

void hideModelIfShow(LiveActor* actor) {
    if (!isHideModel(actor))
        hideModel(actor);
}

bool isExistModel(const LiveActor* actor) {
    return actor->getModelKeeper() != nullptr;
}

void switchShowHideModelIfNearCamera(LiveActor* actor, f32 radius) {
    const sead::Vector3f& trans = al::getTrans(actor);
    const sead::Vector3f& cameraPos = al::getCameraPos(actor, 0);
    bool isInRadius = (trans - cameraPos).length() < radius;
    if (!isInRadius) {
        if (isHideModel(actor)) {
            al::showModel(actor);
            al::onCalcAndDrawEffect(actor);
        }
    } else if (!isHideModel(actor)) {
        al::hideModel(actor);
        al::offCalcAndDrawEffect(actor);
    }
}

bool blinkModel(LiveActor* actor, s32 curFrame, s32 interval, s32 startFrame) {
    if ((curFrame - startFrame) % interval)
        return false;
    if (isHideModel(actor)) {
        showModel(actor);
        return true;
    }
    hideModel(actor);
    return false;
}

void calcViewModelSystem(LiveActor* actor) {
    bool hasCalculatedView = actor->getModelKeeper()->getModelCtrl()->calcView();
    ModelCtrl* modelCtrl = actor->getModelKeeper()->getModelCtrl();
    if (modelCtrl->isNeedUpdate())
        modelCtrl->update();
    else if (!hasCalculatedView)
        return;

    alActorSystemFunction::updateExecutorDraw(actor);
}

void calcViewModel(LiveActor* actor) {
    if (actor->getEffectKeeper())
        actor->getEffectKeeper()->update();
}

void showSilhouetteModel(LiveActor* actor) {
    s32 curActorCount = actor->getSubActorKeeper()->getCurActorCount();
    for (s32 i = 0; i < curActorCount; ++i) {
        SubActorInfo* subActorInfo = actor->getSubActorKeeper()->getSubActorInfo(i);

        if (al::isEqualString(subActorInfo->subActor->getName(), "シルエットモデル"))
            showModelIfHide(subActorInfo->subActor);
        else if (subActorInfo->subActor->getSubActorKeeper())
            al::showSilhouetteModel(subActorInfo->subActor);
    }
}

void hideSilhouetteModel(LiveActor* actor) {
    s32 curActorCount = actor->getSubActorKeeper()->getCurActorCount();
    for (s32 i = 0; i < curActorCount; ++i) {
        SubActorInfo* subActorInfo = actor->getSubActorKeeper()->getSubActorInfo(i);

        if (al::isEqualString(subActorInfo->subActor->getName(), "シルエットモデル"))
            hideModelIfShow(subActorInfo->subActor);
        else if (subActorInfo->subActor->getSubActorKeeper())
            al::hideSilhouetteModel(subActorInfo->subActor);
    }
}

bool isSilhouetteModelHidden(const LiveActor* actor) {
    return isHideModel(
        alSubActorFunction::findSubActor(actor->getSubActorKeeper(), "シルエットモデル"));
}

void showSilhouetteModelIfHide(LiveActor* actor) {
    if (actor->getSubActorKeeper() && isSilhouetteModelHidden(actor))
        showSilhouetteModel(actor);
}

void hideSilhouetteModelIfShow(LiveActor* actor) {
    if (actor->getSubActorKeeper() && !isSilhouetteModelHidden(actor))
        hideSilhouetteModel(actor);
}

void setModelAlphaMask(LiveActor* actor, f32 threshold) {
    ModelCtrl* modelCtrl = actor->getModelKeeper()->getModelCtrl();
    f32 prevAlphaMask = modelCtrl->getAlphaMask();

    bool hasAlphaMask = prevAlphaMask < 1.0f;
    modelCtrl->setModelAlphaMask(threshold);

    if (hasAlphaMask != isModelAlphaMask(actor) || (threshold == 0.0f && prevAlphaMask > 0.0f) ||
        (threshold > 0.0f && prevAlphaMask == 0.0f))
        alActorSystemFunction::updateExecutorDraw(actor);

    if (SubActorKeeper* subActorKeeper = actor->getSubActorKeeper())
        alSubActorFunction::trySyncModelAlphaMask(subActorKeeper, threshold);
}

f32 getModelAlphaMask(const LiveActor* actor) {
    return actor->getModelKeeper()->getModelCtrl()->getAlphaMask();
}

bool isModelAlphaMask(const LiveActor* actor) {
    return actor->getModelKeeper()->getModelCtrl()->getAlphaMask() < 1.0f;
}

void updateModelAlphaMaskCameraDistance(LiveActor* actor, f32 a2, f32 a3, f32 a4, f32 a5) {
    sead::Vector3f cameraLookDir = {0.0f, 0.0f, 0.0f};
    al::calcCameraLookDir(&cameraLookDir, actor, 0);

    const sead::Vector3f& cameraPos = al::getCameraPos(actor, 0);
    const sead::Vector3f& trans = al::getTrans(actor);
    f32 length = ((trans - cameraPos).dot(cameraLookDir) - a3) / (a2 - a3);
    if (length < 0.0f)
        length = 0.0f;
    else if (length > 1.0f)
        length = 1.0f;
    f32 lerpedLength = al::lerpValue(a5, a4, length);
    setModelAlphaMask(actor, lerpedLength);
}

bool isExistZPrePass(const LiveActor* actor) {
    return actor->getModelKeeper() && actor->getModelKeeper()->getModelCtrl()->isExistZPrePass();
}

bool isEnableZPrePass(const LiveActor* actor) {
    if (actor->getModelKeeper()) {
        ModelCtrl* modelCtrl = actor->getModelKeeper()->getModelCtrl();
        return modelCtrl->isValidateZPrePass() && modelCtrl->get_166();
    }
    return false;
}

void validateZPrePass(LiveActor* actor) {
    ModelKeeper* modelKeeper = actor->getModelKeeper();
    if (modelKeeper) {
        ModelCtrl* modelCtrl = modelKeeper->getModelCtrl();
        if (modelCtrl->isExistZPrePass() && !modelCtrl->isValidateZPrePass()) {
            modelCtrl->validateZPrePass();
            modelCtrl->onZPrePass();
            alActorSystemFunction::updateExecutorDraw(actor);
        }
    }
}

void invalidateZPrePass(LiveActor* actor) {
    ModelKeeper* modelKeeper = actor->getModelKeeper();
    if (modelKeeper) {
        ModelCtrl* modelCtrl = modelKeeper->getModelCtrl();
        if (modelCtrl->isExistZPrePass() && modelCtrl->isValidateZPrePass()) {
            modelCtrl->invalidateZPrePass();
            modelCtrl->offZPrePass();
            alActorSystemFunction::updateExecutorDraw(actor);
        }
    }
}

void invalidateOcclusionQuery(LiveActor* actor) {
    ModelCtrl* modelCtrl = actor->getModelKeeper()->getModelCtrl();
    modelCtrl->invalidateOcclusionQuery();
    modelCtrl->recreateDisplayList();
}

void validateOcclusionQuery(LiveActor* actor) {
    ModelCtrl* modelCtrl = actor->getModelKeeper()->getModelCtrl();
    modelCtrl->validateOcclusionQuery();
    modelCtrl->recreateDisplayList();
}

bool isValidOcclusionQuery(const LiveActor* actor) {
    return actor->getModelKeeper()->getModelCtrl()->isValidOcclusionQuery();
}

void setFixedModelFlag(LiveActor* actor) {
    actor->getModelKeeper()->setFixedModelFlag(true);
}

void tryInitFixedModelGpuBuffer(LiveActor* actor) {
    ModelKeeper* mModelKeeper = actor->getModelKeeper();
    if (!mModelKeeper)
        return;

    if (!actor->getExecuteInfo())
        return;

    actor->getModelKeeper()->update();

    sead::Matrix34f baseMtx;
    alActorPoseFunction::calcBaseMtx(&baseMtx, actor);
    al::setBaseMtxAndCalcAnim(actor, baseMtx, al::getScale(actor));

    if (al::isExistDepthShadow(actor))
        actor->getModelKeeper()->getModelCtrl()->updateGpuBufferAll();

    if (mModelKeeper->isFixedModel() &&
        (actor->getEffectKeeper() || actor->getAudioKeeper() || actor->getHitSensorKeeper()))
        al::onUpdateMovementEffectAudioCollisionSensor(actor);
}

void setIgnoreUpdateDrawClipping(LiveActor* actor, bool isIgnoreUpdateDrawClipping) {
    actor->getModelKeeper()->setIgnoreUpdateDrawClipping(isIgnoreUpdateDrawClipping);
}

void setNeedSetBaseMtxAndCalcAnimFlag(LiveActor* actor, bool isNeedSetBaseMtxAndCalcAnim) {
    actor->getModelKeeper()->setNeedSetBaseMtxAndCalcAnimFlag(isNeedSetBaseMtxAndCalcAnim);
}

bool isViewDependentModel(const LiveActor* actor) {
    return actor->getModelKeeper()->getModelCtrl()->getModelObj()->GetViewDependentModelFlags() !=
           0;
}

bool isNeedUpdateModel(const LiveActor* actor) {
    return actor->getModelKeeper()->getModelCtrl()->isNeedUpdateModel();
}

void setEnvTextureMirror(LiveActor* actor, s32 id) {
    EnvTexId envTexId;
    envTexId.mMirrorId = id;

    ModelCtrl* modelCtrl = actor->getModelKeeper()->getModelCtrl();
    s32 numShapes = modelCtrl->getModelObj()->GetNumShapes();
    if (modelCtrl->getModelObj()->GetNumShapes())
        for (s32 i = 0; i < numShapes; ++i)
            modelCtrl->getEnvTexInfo(i).mEnvTexId.change(envTexId);

    modelCtrl->recreateDisplayList();
}

void setEnvTextureProc3D(LiveActor* actor, s32 id) {
    EnvTexId envTexId;
    envTexId.mProc3DId = id;

    ModelCtrl* modelCtrl = actor->getModelKeeper()->getModelCtrl();
    s32 numShapes = modelCtrl->getModelObj()->GetNumShapes();

    for (s32 i = 0; i < numShapes; ++i)
        modelCtrl->getEnvTexInfo(i).mEnvTexId.change(envTexId);

    modelCtrl->recreateDisplayList();
}

void forceApplyCubeMap(LiveActor* actor, const char* cubeMapName) {
    al::forceApplyCubeMap(actor->getModelKeeper(), actor->getSceneInfo()->graphicsSystemInfo,
                          cubeMapName);
}

void setMaterialProgrammable(LiveActor* actor) {
    ModelCtrl* modelCtrl = actor->getModelKeeper()->getModelCtrl();
    s32 numShapes = modelCtrl->getModelObj()->GetNumShapes();

    for (s32 i = 0; i < numShapes; ++i)
        modelCtrl->setMaterialProgrammable(i, true);
}

bool isIncludePrepassCullingShape(LiveActor* actor) {
    ModelCtrl* modelCtrl = actor->getModelKeeper()->getModelCtrl();
    s32 numShapes = modelCtrl->getModelObj()->GetNumShapes();

    for (s32 i = 0; i < numShapes; ++i)
        if (alModelFunction::isEnablePrepassCulling(*modelCtrl->getModelObj(), i))
            return true;
    return false;
}

bool isExistJoint(const LiveActor* actor, const char* jointName) {
    return isExistJoint(actor->getModelKeeper(), jointName);
}

s32 getJointIndex(const LiveActor* actor, const char* jointName) {
    return getJointIndex(actor->getModelKeeper(), jointName);
}

sead::Matrix34f* getJointMtxPtr(const LiveActor* actor, const char* jointName) {
    return getJointMtxPtr(actor->getModelKeeper(), jointName);
}

sead::Matrix34f* getJointMtxPtrByIndex(const LiveActor* actor, s32 index) {
    return getJointMtxPtrByIndex(actor->getModelKeeper(), index);
}

void getJointLocalTrans(sead::Vector3f* out, const LiveActor* actor, const char* jointName) {
    getJointLocalTrans(out, actor->getModelKeeper(), jointName);
}

void calcJointPos(sead::Vector3f* out, const LiveActor* actor, const char* jointName) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    jointMtx->getTranslation(*out);
}

void calcJointOffsetPos(sead::Vector3f* out, const LiveActor* actor, const char* jointName,
                        const sead::Vector3f& offsetPos) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    sead::Vector3CalcCommon<f32>::mul(*out, *jointMtx, offsetPos);
}

void calcJointPosByIndex(sead::Vector3f* out, const LiveActor* actor, s32 index) {
    sead::Matrix34f* jointMtx = getJointMtxPtrByIndex(actor, index);
    jointMtx->getTranslation(*out);
}

void calcJointSideDir(sead::Vector3f* out, const LiveActor* actor, const char* jointName) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    jointMtx->getBase(*out, 0);
    al::tryNormalizeOrZero(out);
}

void calcJointUpDir(sead::Vector3f* out, const LiveActor* actor, const char* jointName) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    jointMtx->getBase(*out, 1);
    al::tryNormalizeOrZero(out);
}

void calcJointFrontDir(sead::Vector3f* out, const LiveActor* actor, const char* jointName) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    jointMtx->getBase(*out, 2);
    al::tryNormalizeOrZero(out);
}

void calcJointScale(sead::Vector3f* out, const LiveActor* actor, const char* jointName) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    al::calcMtxScale(out, *jointMtx);
}

void calcJointQuat(sead::Quatf* out, const LiveActor* actor, const char* jointName) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    jointMtx->toQuat(*out);
    out->normalize();
}

void multVecJointMtx(sead::Vector3f* out, const sead::Vector3f& multVec, const LiveActor* actor,
                     const char* jointName) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    sead::Vector3CalcCommon<f32>::mul(*out, *jointMtx, multVec);
}

void multVecJointInvMtx(sead::Vector3f* out, const sead::Vector3f& multVec, const LiveActor* actor,
                        const char* jointName) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);

    sead::Matrix34f inverseMtx;
    sead::Matrix34CalcCommon<f32>::inverse(inverseMtx, *jointMtx);
    sead::Vector3CalcCommon<f32>::mul(*out, inverseMtx, multVec);
}

void multMtxJointInvMtx(sead::Matrix34f* out, const sead::Matrix34f& multMtx,
                        const LiveActor* actor, const char* jointName) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);

    sead::Matrix34f inverseMtx;
    sead::Matrix34CalcCommon<f32>::inverse(inverseMtx, *jointMtx);
    *out = inverseMtx * multMtx;
}

void setJointVisibility(LiveActor* actor, const char* jointName, bool a3) {
    setJointVisibility(actor->getModelKeeper(), jointName, a3);
    alActorSystemFunction::updateExecutorDraw(actor);
}

bool isJointVisibility(const LiveActor* actor, const char* jointName) {
    return isJointVisibility(actor->getModelKeeper(), jointName);
}

bool isFaceJointXDirDegreeYZ(const LiveActor* actor, const char* jointName,
                             const sead::Vector3f& dir, f32 y, f32 z) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    sead::Vector3f side;
    jointMtx->getBase(side, 0);
    sead::Vector3f up;
    jointMtx->getBase(up, 1);
    sead::Vector3f pos;
    jointMtx->getTranslation(pos);
    pos = dir - pos;

    return al::isNearAngleDegreeHV(pos, side, up, y, z);
}

bool isFaceJointYDirDegreeZX(const LiveActor* actor, const char* jointName,
                             const sead::Vector3f& dir, f32 z, f32 x) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    sead::Vector3f up;
    jointMtx->getBase(up, 1);
    sead::Vector3f front;
    jointMtx->getBase(front, 2);
    sead::Vector3f pos;
    jointMtx->getTranslation(pos);
    pos = dir - pos;

    return al::isNearAngleDegreeHV(pos, up, front, z, x);
}

bool isFaceJointZDirDegreeXY(const LiveActor* actor, const char* jointName,
                             const sead::Vector3f& dir, f32 x, f32 y) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    sead::Vector3f front;
    jointMtx->getBase(front, 2);
    sead::Vector3f side;
    jointMtx->getBase(side, 0);
    sead::Vector3f pos;
    jointMtx->getTranslation(pos);
    pos = dir - pos;

    return al::isNearAngleDegreeHV(pos, front, side, x, y);
}

void calcJointAngleXDirToTargetOnYDir(const LiveActor* actor, const char* jointName,
                                      const sead::Vector3f& dir) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    al::calcMtxLocalDirAngleOnPlaneToTarget(jointMtx, dir, 0, 1);
}

void calcJointAngleXDirToTargetOnZDir(const LiveActor* actor, const char* jointName,
                                      const sead::Vector3f& dir) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    al::calcMtxLocalDirAngleOnPlaneToTarget(jointMtx, dir, 0, 2);
}

void calcJointAngleYDirToTargetOnXDir(const LiveActor* actor, const char* jointName,
                                      const sead::Vector3f& dir) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    al::calcMtxLocalDirAngleOnPlaneToTarget(jointMtx, dir, 1, 0);
}

void calcJointAngleYDirToTargetOnZDir(const LiveActor* actor, const char* jointName,
                                      const sead::Vector3f& dir) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    al::calcMtxLocalDirAngleOnPlaneToTarget(jointMtx, dir, 1, 2);
}

void calcJointAngleZDirToTargetOnXDir(const LiveActor* actor, const char* jointName,
                                      const sead::Vector3f& dir) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    al::calcMtxLocalDirAngleOnPlaneToTarget(jointMtx, dir, 2, 0);
}

void calcJointAngleZDirToTargetOnYDir(const LiveActor* actor, const char* jointName,
                                      const sead::Vector3f& dir) {
    sead::Matrix34f* jointMtx = getJointMtxPtr(actor, jointName);
    al::calcMtxLocalDirAngleOnPlaneToTarget(jointMtx, dir, 2, 1);
}

void getMaterialName(const LiveActor* actor, s32 a2) {
    getMaterialName(actor->getModelKeeper(), a2);
}

void getMaterialCount(const LiveActor* actor) {
    getMaterialCount(actor->getModelKeeper());
}

bool isExistMaterial(const LiveActor* actor, const char* materialName) {
    return getMaterialIndex(actor->getModelKeeper(), materialName) >= 0;
}

nn::g3d::MaterialObj* getMaterialObj(const LiveActor* actor, s32 a2) {
    return actor->getModelKeeper()->getModelCtrl()->getModelObj()->GetMaterial(a2);
}

nn::g3d::MaterialObj* getMaterialObj(const LiveActor* actor, const char* materialName) {
    return actor->getModelKeeper()->getModelCtrl()->getModelObj()->FindMaterial(materialName);
}

// s32 getMaterialIndex(const LiveActor* actor, const char* materialName);
// bool isExistMaterialTexture(const LiveActor* actor, const char*, const char*);
const char* getMaterialCategory(const LiveActor* actor, s32 materialIndex) {
    s32 index = actor->getModelKeeper()
                    ->getModelCtrl()
                    ->getModelMaterialCategory()
                    ->getCategoryIdFromMaterialIndex(materialIndex);
    ActorSceneInfo* sceneInfo = actor->getSceneInfo();
    return sceneInfo->graphicsSystemInfo->getMaterialCategoryKeeper()->getCategoryName(index);
}

const char* tryGetMaterialCategory(const LiveActor* actor, s32 materialIndex) {
    ModelMaterialCategory* category =
        actor->getModelKeeper()->getModelCtrl()->getModelMaterialCategory();
    if (!category)
        return nullptr;

    s32 index = category->getCategoryIdFromMaterialIndex(materialIndex);

    ActorSceneInfo* sceneInfo = actor->getSceneInfo();
    return sceneInfo->graphicsSystemInfo->getMaterialCategoryKeeper()->getCategoryName(index);
}

bool isOnlyMaterialCategoryObject(const LiveActor* actor) {
    s32 materialCount = al::getMaterialCount(actor->getModelKeeper());
    for (s32 i = 0; i < materialCount; ++i) {
        const char* category = al::getMaterialCategory(actor, i);
        if (!al::isEqualString(category, "Obj"))
            return false;
    }
    return true;
}

void showMaterial(LiveActor* actor, const char* materialName) {
    al::showMaterial(actor->getModelKeeper(), materialName);
    alActorSystemFunction::updateExecutorDraw(actor);
}

void hideMaterial(LiveActor* actor, const char* materialName) {
    al::hideMaterial(actor->getModelKeeper(), materialName);
    alActorSystemFunction::updateExecutorDraw(actor);
}

void showMaterial(LiveActor* actor, s32 index) {
    al::showMaterial(actor->getModelKeeper(), index);
    alActorSystemFunction::updateExecutorDraw(actor);
}

void hideMaterial(LiveActor* actor, s32 index) {
    al::hideMaterial(actor->getModelKeeper(), index);
    alActorSystemFunction::updateExecutorDraw(actor);
}

void showMaterialAll(LiveActor* actor) {
    s32 materialCount = al::getMaterialCount(actor->getModelKeeper());
    for (s32 i = 0; i < materialCount; ++i)
        al::showMaterial(actor, i);
}

bool tryShowMaterial(LiveActor* actor, s32 index) {
    ModelKeeper* modelKeeper = actor->getModelKeeper();
    if (!modelKeeper)
        return false;
    al::showMaterial(actor, index);
    return true;
}

bool tryHideMaterial(LiveActor* actor, s32 index) {
    ModelKeeper* modelKeeper = actor->getModelKeeper();
    if (!modelKeeper)
        return false;
    al::hideMaterial(actor, index);
    return true;
}

bool tryShowMaterialAll(LiveActor* actor) {
    ModelKeeper* modelKeeper = actor->getModelKeeper();
    if (!modelKeeper)
        return false;
    s32 materialCount = al::getMaterialCount(actor->getModelKeeper());
    for (s32 i = 0; i < materialCount; ++i)
        al::showMaterial(actor, i);
    return true;
}

// void setModelMaterialParameterF32(const LiveActor* actor, s32, const char*, f32);
// void setModelMaterialParameterF32(const LiveActor* actor, const char*, const char*, f32);
// void setModelMaterialParameterV2F(const LiveActor* actor, s32, const char*, const
// sead::Vector2f&); void setModelMaterialParameterV2F(const LiveActor* actor, const char*, const
// char*,
//                                   const sead::Vector2f&);
// void setModelMaterialParameterV3F(const LiveActor* actor, s32, const char*, const
// sead::Vector3f&); void setModelMaterialParameterV3F(const LiveActor* actor, const char*, const
// char*,
//                                   const sead::Vector3f&);
// void setModelMaterialParameterV4F(const LiveActor* actor, s32, const char*, const
// sead::Vector4f&); void setModelMaterialParameterV4F(const LiveActor* actor, const char*, const
// char*,
//                                   const sead::Vector4f&);
// void setModelMaterialParameterRgb(const LiveActor* actor, s32, const char*, const
// sead::Vector3f&); void setModelMaterialParameterRgb(const LiveActor* actor, const char*, const
// char*,
//                                   const sead::Vector3f&);
// void setModelMaterialParameterRgb(const LiveActor* actor, s32, const char*, const
// sead::Color4f&); void setModelMaterialParameterRgb(const LiveActor* actor, const char*, const
// char*,
//                                   const sead::Color4f&);
// void setModelMaterialParameterRgba(const LiveActor* actor, s32, const char*, const
// sead::Color4f&); void setModelMaterialParameterRgba(const LiveActor* actor, const char*, const
// char*,
//                                    const sead::Color4f&);
// void setModelMaterialParameterAlpha(const LiveActor* actor, s32, const char*, f32);
// void setModelMaterialParameterAlpha(const LiveActor* actor, const char*, const char*, f32);
// void setModelMaterialParameterTextureTrans(const LiveActor* actor, const char*, s32,
//                                            const sead::Vector2f&);
// void getModelMaterialParameterDisplacementScale(const LiveActor* actor, const char*, s32);
// void setModelMaterialParameterDisplacementScale(const LiveActor* actor, const char*, s32, f32);
// void getModelUniformBlock(const LiveActor* actor, const char*);
// void findModelUniformBlock(const LiveActor* actor, const char*);
// void swapModelUniformBlock(agl::UniformBlock*);
// void flushModelUniformBlock(agl::UniformBlock*);
// void getModelDrawCategoryFromShaderAssign(bool*, bool*, bool*, bool*, const LiveActor* actor);
// void trySetOcclusionQueryBox(LiveActor* actor, f32);
// void trySetOcclusionQueryBox(LiveActor* actor, const sead::BoundBox3f&);
// void trySetOcclusionQueryCenter(LiveActor* actor, const sead::Vector3f*);
const char* getModelName(const LiveActor* actor) {
    return actor->getModelKeeper()->getName();
}

bool isModelName(const LiveActor* actor, const char* name) {
    return al::isEqualString(getModelName(actor), name);
}

f32 calcModelBoundingSphereRadius(const LiveActor* actor) {
    actor->getModelKeeper()->getModelCtrl()->calcBoundingLod(0);
    return alModelFunction::calcBoundingSphere(actor->getModelKeeper()->getModelCtrl());
}

void getBoundingSphereCenterAndRadius(sead::Vector3f* center, f32* radius, const LiveActor* actor) {
    const nn::g3d::Bounds& sphere =
        actor->getModelKeeper()->getModelCtrl()->getModelObj()->GetBounds();
    center->x = sphere.x;
    center->y = sphere.y;
    center->z = sphere.z;
    *radius = sphere.radius;
}

void calcModelBoundingBox(sead::BoundBox3f* out, const LiveActor* actor) {
    alModelFunction::calcBoundingBox(out, actor->getModelKeeper()->getModelCtrl());
}

void calcModelBoundingBoxMtx(sead::Matrix34f* out, const LiveActor* actor) {
    alModelFunction::calcBoundingBoxMtx(out, actor->getModelKeeper()->getModelCtrl());
}

void submitViewModel(const LiveActor* actor, const sead::Matrix34f& mtx) {}

// void replaceMaterialTextureRef(LiveActor* actor, nn::g3d::TextureRef*, const char*, const char*);
// void replaceMaterialResTexture(LiveActor* actor, nn::gfx::ResTexture*, const char*, const char*);
// void replaceMaterialResTexture(LiveActor*, const char*, const char*, nn::gfx::DescriptorSlot,
// const nn::gfx::TextureView*); void replaceMaterialLayoutTexture(LiveActor* actor, const
// LayoutTextureRenderObj*, const char*, const char*);

void recreateModelDisplayList(const LiveActor* actor) {
    actor->getModelKeeper()->getModelCtrl()->recreateDisplayList();
}

s32 calcPolygonNum(const LiveActor* actor, s32 index) {
    ModelKeeper* modelKeeper = actor->getModelKeeper();
    if (modelKeeper)
        return al::calcPolygonNum(modelKeeper, index);
    return 0;
}

s32 calcPolygonNumCurrentLod(const LiveActor* actor) {
    ModelKeeper* modelKeeper = actor->getModelKeeper();
    if (modelKeeper)
        return al::calcPolygonNumCurrentLod(modelKeeper);
    return 0;
}

// s32 calcPolygonNumCurrentLodWithoutVisAnim(const LiveActor* actor);

s32 getLodLevel(const LiveActor* actor) {
    return al::getLodLevel(actor->getModelKeeper());
}

s32 getMaterialLodLevel(const LiveActor* actor) {
    return al::getMaterialLodLevel(actor->getModelKeeper());
}

s32 getLodLevelNoClamp(const LiveActor* actor) {
    return al::getLodLevelNoClamp(actor->getModelKeeper());
}

s32 getLodModelCount(const LiveActor* actor) {
    return alModelFunction::getLodModelCount(
        actor->getModelKeeper()->getModelCtrl()->getModelObj());
}

void forceLodLevel(LiveActor* actor, s32 level) {
    actor->getModelKeeper()->getModelCtrl()->setLodLevelForce(level);
}

void unforceLodLevel(LiveActor* actor) {
    actor->getModelKeeper()->getModelCtrl()->setLodLevelForce(-1);
}

bool isExistLodModel(const LiveActor* actor) {
    ModelKeeper* modelKeeper = actor->getModelKeeper();
    if (modelKeeper)
        return alModelFunction::isExistLodModel(modelKeeper->getModelCtrl()->getModelObj());
    return false;
}

bool isEnableMaterialLod(const LiveActor* actor) {
    return al::isEnableMaterialLod(actor->getModelKeeper());
}

void validateLodModel(LiveActor* actor) {
    al::validateLodModel(actor->getModelKeeper());
}

void invalidateLodModel(LiveActor* actor) {
    al::invalidateLodModel(actor->getModelKeeper());
}

bool isValidateLodModel(const LiveActor* actor) {
    if (isExistLodModel(actor))
        return al::isValidateLodModel(actor->getModelKeeper());
    return false;
}

bool isExistDitherAnimator(const LiveActor* actor) {
    return actor->getModelKeeper()->getModelCtrl()->getActorDitherAnimator() != nullptr;
}

bool isValidNearDitherAnim(const LiveActor* actor) {
    return actor->getModelKeeper()->getModelCtrl()->getActorDitherAnimator()->isValidNearClip();
}

void stopDitherAnimAutoCtrl(LiveActor* actor) {
    actor->getModelKeeper()->getModelCtrl()->getActorDitherAnimator()->stopAutoCtrl();
}

void restartDitherAnimAutoCtrl(LiveActor* actor) {
    actor->getModelKeeper()->getModelCtrl()->getActorDitherAnimator()->restartAutoCtrl();
}

void validateDitherAnim(LiveActor* actor) {
    actor->getModelKeeper()->getModelCtrl()->getActorDitherAnimator()->validateDitherAnim();
}

void invalidateDitherAnim(LiveActor* actor) {
    actor->getModelKeeper()->getModelCtrl()->getActorDitherAnimator()->invalidateDitherAnim();
}

// void validateFarDitherIfInvalidateClipping(LiveActor* actor);

void setDitherAnimSphereRadius(LiveActor* actor, f32);
void setDitherAnimBoundingBox(LiveActor* actor, const sead::Vector3f&);
void setDitherAnimMaxAlpha(LiveActor* actor, f32);
void setDitherAnimClippingJudgeLocalOffset(LiveActor* actor, const sead::Vector3f&);
void setDitherAnimClippingJudgeParam(LiveActor* actor, const char*);
void resetDitherAnimClippingJudgeParam(LiveActor* actor);
void getDitherAnimMinNearDitherAlpha(const LiveActor* actor);
f32 getDitherAnimNearClipStartDistance(const LiveActor* actor);
f32 getDitherAnimNearClipEndDistance(const LiveActor* actor);
void calcDitherAnimJudgeDistance(const LiveActor* actor);
void createUniqueShader(LiveActor* actor);
bool isJudgedToClipFrustum(const ClippingDirector*, const sead::Vector3f&, f32, f32);
bool isJudgedToClipFrustum(const LiveActor* actor, const sead::Vector3f&, f32, f32);
bool isJudgedToClipFrustum(const LiveActor* actor, f32, f32);
bool isJudgedToClipFrustumWithoutFar(const ClippingDirector*, const sead::Vector3f&, f32, f32);
bool isJudgedToClipFrustumWithoutFar(const LiveActor* actor, const sead::Vector3f&, f32, f32);
bool isJudgedToClipFrustumWithoutFar(const LiveActor* actor, f32, f32);
}  // namespace al