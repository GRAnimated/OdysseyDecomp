#include "Player/PlayerHackStartShaderCtrl.h"

#include "Library/Draw/GraphicsFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/MatrixUtil.h"
#include "Library/Model/ModelCtrl.h"
#include "Library/Model/ModelKeeper.h"

// NON_MATCHING: fields and initialization behavior are recovered, but layout/call order differs; next
// source-level hypothesis is the original parameter initialization helper shape.
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
            al::calcModelBoundingBox(&box, mParent);
            mParam->_4 = box.getSizeY();
        }
    }
}

// NON_MATCHING: host/model setup behavior is recovered, but call/register order differs; next
// source-level hypothesis is the original shared setup helper shape.
void PlayerHackStartShaderCtrl::setHost(al::LiveActor* host) {
    mParent = host;
    if (al::isExistModel(mParent)) {
        al::setMaterialProgrammable(mParent);
        if (mParam->_4 == -1.0f) {
            sead::BoundBox3f box;
            al::calcModelBoundingBox(&box, mParent);
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

// NON_MATCHING: shader projection behavior is recovered, but matrix temporaries and call order differ;
// next source-level hypothesis is the original projection helper signature and local layout.
void PlayerHackStartShaderCtrl::update() {
    if (!mIsActive)
        return;

    mTime++;
    if (mTime < mParam->_8)
        return;

    if (mTime == mParam->_8)
        alGraphicsFunction::requestChangeShaderVariation(mParent, "enable_compose_capture", "1",
                                                         true);

    const f32 rate = (f32)(mTime - mParam->_8) / (f32)mParam->_c;
    sead::Vector3f trans = al::getTrans(mParent);
    f32 startHeight = mParam->_4;
    f32 endHeight = 0.0f;
    if (mParam->_0) {
        startHeight *= 0.5f;
        endHeight = startHeight * -1.0f;
    }
    trans.y += al::lerpValue(startHeight, endHeight, rate);

    sead::Matrix44f projection;
    al::makeMtxProjFromQuatPoseFront(&projection, mQuat, sead::Vector2f(8.0f, 128.0f), trans);
    mParent->getModelKeeper()->getModelCtrl()->setModelProgProjMtx0(projection);

    mColor.a = 1.0f - rate;
    const s32 materialCount = al::getMaterialCount(mParent);
    for (s32 i = 0; i < materialCount; i++)
        al::setModelMaterialParameterRgba(mParent, i, "hack_color", mColor);

    if (mTime >= mParam->_8 + mParam->_c && mIsActive) {
        mIsActive = false;
        alGraphicsFunction::requestChangeShaderVariation(mParent, "enable_compose_capture", "0",
                                                         true);
    }
}

void PlayerHackStartShaderCtrl::end() {
    if (mIsActive) {
        mIsActive = false;
        alGraphicsFunction::requestChangeShaderVariation(mParent, "enable_compose_capture", "0",
                                                         true);
    }
}
