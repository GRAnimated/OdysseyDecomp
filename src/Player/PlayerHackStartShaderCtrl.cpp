#include "Player/PlayerHackStartShaderCtrl.h"

#include "Library/Draw/GraphicsFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/MatrixUtil.h"
#include "Library/Model/ModelCtrl.h"
#include "Library/Model/ModelKeeper.h"

namespace alModelFunction {
void calcBoundingBox(sead::BoundBox3f*, const al::ModelCtrl*);
}

namespace {
const sead::Vector2f _18D9E40(8.0f, 128.0f);
}

PlayerHackStartShaderCtrl::PlayerHackStartShaderCtrl(al::LiveActor* host,
                                                     PlayerHackStartShaderParam* param)
    : mParent(host),
      mTime(0),
      mIsActive(false),
      mColor(1.0f, 0.5f, 0.15f, 1.0f),
      mQuat(sead::Quatf::unit),
      mParam(param) {
    if (!mParam)
        mParam = new PlayerHackStartShaderParam(false, -1.0f, 10, 20);
    if (mParent && al::isExistModel(mParent)) {
        al::setMaterialProgrammable(mParent);
        if (mParam->_4 == -1.0f) {
            sead::BoundBox3f box;
            alModelFunction::calcBoundingBox(&box, mParent->getModelKeeper()->getModelCtrl());
            mParam->_4 = box.getSizeY();
        }
    }
}

void PlayerHackStartShaderCtrl::setHost(al::LiveActor* host) {
    mParent = host;
    if (al::isExistModel(mParent)) {
        al::setMaterialProgrammable(mParent);
        if (mParam->_4 == -1.0f) {
            sead::BoundBox3f box;
            alModelFunction::calcBoundingBox(&box, mParent->getModelKeeper()->getModelCtrl());
            mParam->_4 = box.getSizeY();
        }
    }
}

void PlayerHackStartShaderCtrl::start() {
    if (!al::isExistModel(mParent))
        return;

    mIsActive = true;
    mTime = 0;
    sead::Vector3f front = sead::Vector3f::ez;
    sead::Vector3f up = sead::Vector3f::ey;
    al::calcFrontDir(&front, mParent);
    al::calcUpDir(&up, mParent);
    al::makeQuatUpFront(&mQuat, up, front);
}

// NON_MATCHING: target/current size and all instructions match except the height flag uses W9 in the target versus W8 currently; next source-level hypothesis is a source lifetime that keeps the parameter pointer live across the flag load.
void PlayerHackStartShaderCtrl::update() {
    if (!mIsActive)
        return;

    mTime++;
    if (mParam->_8 <= mTime) {
        if (mParam->_8 == mTime)
            alGraphicsFunction::requestChangeShaderVariation(mParent, "enable_compose_capture", "1",
                                                             true);

        const f32 rate = (f32)(mTime - mParam->_8) / (f32)mParam->_c;
        al::ModelCtrl* modelCtrl = mParent->getModelKeeper()->getModelCtrl();
        sead::Matrix44f projection;
        sead::Vector3f trans = al::getTrans(mParent);
        const f32 startHeight = mParam->_4;
        if (mParam->_0)
            trans.y += al::lerpValue(startHeight * 0.5f, startHeight * -0.5f, rate);
        else
            trans.y += al::lerpValue(startHeight, 0.0f, rate);

        al::makeMtxProjFromQuatPoseFront(&projection, mQuat, _18D9E40, trans);
        modelCtrl->setModelProgProjMtx0(projection);

        mColor.a = 1.0f - rate;
        const s32 materialCount = al::getMaterialCount(mParent);
        for (s32 i = 0; i < materialCount; i++)
            al::setModelMaterialParameterRgba(mParent, i, "hack_color", mColor);

        if (mTime >= mParam->_c + mParam->_8 && mIsActive) {
            mIsActive = false;
            alGraphicsFunction::requestChangeShaderVariation(mParent, "enable_compose_capture", "0",
                                                             true);
        }
    }
}

void PlayerHackStartShaderCtrl::end() {
    if (mIsActive) {
        mIsActive = false;
        alGraphicsFunction::requestChangeShaderVariation(mParent, "enable_compose_capture", "0",
                                                         true);
    }
}
