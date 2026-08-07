#include "Player/PlayerFormSensorCollisionArranger.h"

#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Math/MathUtil.h"

#include "Player/IPlayerModelChanger.h"
#include "Player/PlayerColliderHakoniwa.h"
#include "Player/PlayerHackKeeper.h"
#include "Util/PlayerCollisionUtil.h"

namespace CollisionShapeUtil {
void setShapeOffsetAllArrow(IUsePlayerCollision*, const sead::Vector3f&);
}

namespace {
const char* const sHeadSensorNames2D[] = {
    "2DHead", "2DSquatHead", "2DHead", "2DHead", "2DHead", "2DHead",
    "2DHead", "2DHead",      "2DHead", "2DHead", "2DHead",
};

const char* const sHeadSensorNames3D[] = {
    "Head",         "SquatHead",    "Head", "WallGrabHead", "WallGrabHead", "PoleClimbHead",
    "Head",         "Head",         "Head", "Head",         "Head",
};
}  // namespace

PlayerFormSensorCollisionArranger::PlayerFormSensorCollisionArranger(
    al::LiveActor* player, PlayerColliderHakoniwa* collider,
    const IPlayerModelChanger* modelChanger, const PlayerHackKeeper* hackKeeper)
    : mPlayer(player), mCollider(collider), mModelChanger(modelChanger), mHackKeeper(hackKeeper),
      mModelForm(0), mActionForm(0), mAttackSensorForm(0), mIsFormDirty(false),
      mActionFront(0.0f, 0.0f, 0.0f), mIsBind(false) {}

void PlayerFormSensorCollisionArranger::setFormModel3D() {
    if (mModelForm != 1) {
        mModelForm = 1;
        mIsFormDirty = true;
    }
}

void PlayerFormSensorCollisionArranger::setFormModel2D() {
    if (mModelForm != 2) {
        mModelForm = 2;
        mIsFormDirty = true;
    }
}

void PlayerFormSensorCollisionArranger::setFormActionSquat() {
    if (mActionForm != 2) {
        mActionForm = 2;
        mIsFormDirty = true;
    }
}

void PlayerFormSensorCollisionArranger::setFormActionStandup() {
    if (mActionForm != 1) {
        mActionForm = 1;
        mIsFormDirty = true;
    }
}

void PlayerFormSensorCollisionArranger::setFormActionWallGrab(const sead::Vector3f& front) {
    if (mActionForm != 4 || !al::isNearDirection(mActionFront, front, 0.01f)) {
        mActionForm = 4;
        mActionFront = front;
        mIsFormDirty = true;
    }
}

void PlayerFormSensorCollisionArranger::setFormActionGrabCeil(const sead::Vector3f& front) {
    if (mActionForm != 5 || !al::isNearDirection(mActionFront, front, 0.01f)) {
        mActionForm = 5;
        mActionFront = front;
        mIsFormDirty = true;
    }
}

void PlayerFormSensorCollisionArranger::setFormActionPoleClimb(const sead::Vector3f& front) {
    if (mActionForm != 6 || !al::isNearDirection(mActionFront, front, 0.01f)) {
        mActionForm = 6;
        mActionFront = front;
        mIsFormDirty = true;
    }
}

void PlayerFormSensorCollisionArranger::setFormActionSwim() {
    if (mActionForm != 3) {
        mActionForm = 3;
        mIsFormDirty = true;
    }
}

void PlayerFormSensorCollisionArranger::setFormActionHack() {
    if (mActionForm != 7) {
        mActionForm = 7;
        mIsFormDirty = true;
    }
}

void PlayerFormSensorCollisionArranger::setFormActionBind(bool isBind) {
    if (mActionForm != 8 || mIsBind != isBind) {
        mActionForm = 8;
        mIsBind = isBind;
        mIsFormDirty = true;
    }
}

void PlayerFormSensorCollisionArranger::setFormActionRecovery() {
    if (mActionForm != 9) {
        mActionForm = 9;
        mIsFormDirty = true;
    }
}

void PlayerFormSensorCollisionArranger::setFormActionAbyss(const sead::Vector3f& front) {
    if (mActionForm != 10 || !al::isNearDirection(mActionFront, front, 0.01f)) {
        mActionForm = 10;
        mActionFront = front;
        mIsFormDirty = true;
    }
}

void PlayerFormSensorCollisionArranger::setFormActionDead() {
    if (mActionForm != 11) {
        mActionForm = 11;
        mIsFormDirty = true;
    }
}

void PlayerFormSensorCollisionArranger::setFormAttackSensorNone() {
    if (mAttackSensorForm != 0) {
        mAttackSensorForm = 0;
        mIsFormDirty = true;
    }
}

void PlayerFormSensorCollisionArranger::setFormAttackSensorSpin() {
    if (mAttackSensorForm != 1) {
        mAttackSensorForm = 1;
        mIsFormDirty = true;
    }
}

void PlayerFormSensorCollisionArranger::setFormAttackSensorTornado() {
    if (mAttackSensorForm != 2) {
        mAttackSensorForm = 2;
        mIsFormDirty = true;
    }
}

void PlayerFormSensorCollisionArranger::setCollisionShapeOffsetGround(f32 offset) {
    if (!mModelChanger->is2DModel()) {
        if (al::isNearZero(offset, 0.001f))
            mCollider->validateGroundSupport();
        else
            mCollider->invalidateGroundSupport();

        CollisionShapeUtil::setShapeOffsetAllArrow(mCollider, offset * sead::Vector3f::ey);
    }
}

// NON_MATCHING: common-result dispatch mirrors the target outer branch; next compare unsigned range lowering.
const char* PlayerFormSensorCollisionArranger::getHeadSensorName() const {
    const char* sensorName = nullptr;
    if (mModelForm != 1) {
        if (mModelForm == 2) {
            const s32 index = mActionForm - 1;
            if (static_cast<u32>(index) < 11 &&
                ((0x783u >> static_cast<u16>(index)) & 1) != 0)
                sensorName = sHeadSensorNames2D[index];
        }
    } else {
        const s32 index = mActionForm - 1;
        if (static_cast<u32>(index) < 11)
            sensorName = sHeadSensorNames3D[index];
    }
    return sensorName;
}

// NON_MATCHING: exact-size helper-inline form differs only in unsigned range compare encoding (#10/BHI vs #11/BHS).
const sead::Vector3f& PlayerFormSensorCollisionArranger::getHeadPos() const {
    return al::getSensorPos(al::getHitSensor(mPlayer, getHeadSensorName()));
}

// NON_MATCHING: exact-size helper-inline form differs only in unsigned range compare encoding (#10/BHI vs #11/BHS).
f32 PlayerFormSensorCollisionArranger::getHeadRadius() const {
    return al::getSensorRadius(al::getHitSensor(mPlayer, getHeadSensorName()));
}

// NON_MATCHING: direct-return switches are four bytes under target and reverse the outer model branch; next test a validator-clean common sensor-name dispatch.
const sead::Vector3f& PlayerFormSensorCollisionArranger::getBodyPos() const {
    switch (mModelForm) {
    case 1:
        switch (mActionForm) {
        case 1:
        case 3:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
            return al::getSensorPos(al::getHitSensor(mPlayer, "Body"));
        case 2:
            return al::getSensorPos(al::getHitSensor(mPlayer, "SquatBody"));
        case 4:
        case 5:
            return al::getSensorPos(al::getHitSensor(mPlayer, "WallGrabBody"));
        case 6:
            return al::getSensorPos(al::getHitSensor(mPlayer, "PoleClimbBody"));
        }
        break;
    case 2:
        switch (mActionForm) {
        case 1:
        case 8:
        case 9:
        case 10:
        case 11:
            return al::getSensorPos(al::getHitSensor(mPlayer, "2DBody"));
        case 2:
            return al::getSensorPos(al::getHitSensor(mPlayer, "2DSquatBody"));
        }
        break;
    }
    return sead::Vector3f::zero;
}

bool PlayerFormSensorCollisionArranger::isEnableSafetyPointForm() const {
    return (mActionForm & ~1) != 4;
}

void PlayerFormSensorCollisionArranger::update() {
    if (mIsFormDirty)
        syncForm();
}

// NON_MATCHING: complete source is 16 bytes under target because the compiler reverses the target sensor-switch layout; next test validator-clean nested conditionals.
void PlayerFormSensorCollisionArranger::syncForm() {
    al::invalidateHitSensors(mPlayer);

    s32 modelForm = mModelForm;
    s32 actionForm = mActionForm;
    if (modelForm == 2) {
        switch (actionForm) {
        case 1:
        case 9:
            al::validateHitSensor(mPlayer, "2DBody");
            al::validateHitSensor(mPlayer, "2DHead");
            al::validateHitSensor(mPlayer, "2DFoot");
            al::validateHitSensor(mPlayer, "Eye");
            validateAttackSensor();
            break;
        case 2:
            al::validateHitSensor(mPlayer, "2DSquatBody");
            al::validateHitSensor(mPlayer, "2DSquatHead");
            al::validateHitSensor(mPlayer, "2DSquatFoot");
            al::validateHitSensor(mPlayer, "Eye");
            validateAttackSensor();
            break;
        case 8:
            if (mIsBind) {
                al::validateHitSensor(mPlayer, "Body");
                al::validateHitSensor(mPlayer, "Head");
                al::validateHitSensor(mPlayer, "Foot");
                al::validateHitSensor(mPlayer, "HipDropKnockDown");
                al::validateHitSensor(mPlayer, "Eye");
                validateAttackSensor();
            }
            break;
        }
    } else if (modelForm == 1) {
        switch (actionForm) {
        case 1:
        case 3:
        case 9:
            al::validateHitSensor(mPlayer, "Carry");
            al::validateHitSensor(mPlayer, "Body");
            al::validateHitSensor(mPlayer, "Head");
            al::validateHitSensor(mPlayer, "Foot");
            al::validateHitSensor(mPlayer, "HipDropKnockDown");
            al::validateHitSensor(mPlayer, "Eye");
            al::validateHitSensor(mPlayer, "Carry");
            validateAttackSensor();
            break;
        case 2:
            al::validateHitSensor(mPlayer, "SquatBody");
            al::validateHitSensor(mPlayer, "SquatHead");
            al::validateHitSensor(mPlayer, "SquatFoot");
            al::validateHitSensor(mPlayer, "HipDropKnockDown");
            al::validateHitSensor(mPlayer, "Eye");
            validateAttackSensor();
            break;
        case 4:
        case 5:
            al::validateHitSensor(mPlayer, "WallGrabBody");
            al::validateHitSensor(mPlayer, "WallGrabHead");
            al::validateHitSensor(mPlayer, "WallGrabFoot");
            al::validateHitSensor(mPlayer, "Eye");
            validateAttackSensor();
            break;
        case 6:
            al::validateHitSensor(mPlayer, "PoleClimbBody");
            al::validateHitSensor(mPlayer, "PoleClimbHead");
            al::validateHitSensor(mPlayer, "PoleClimbFoot");
            al::validateHitSensor(mPlayer, "Eye");
            validateAttackSensor();
            break;
        case 8:
            if (mIsBind) {
                al::validateHitSensor(mPlayer, "Body");
                al::validateHitSensor(mPlayer, "Head");
                al::validateHitSensor(mPlayer, "Foot");
                al::validateHitSensor(mPlayer, "HipDropKnockDown");
                al::validateHitSensor(mPlayer, "Eye");
                validateAttackSensor();
            }
            break;
        }
    }

    modelForm = mModelForm;
    actionForm = mActionForm;

    switch (actionForm) {
    case 1:
    case 8:
        if (modelForm == 2)
            mCollider->changeCollision2DNormal();
        else
            mCollider->changeCollisionNormal();
        break;
    case 2:
        if (modelForm == 2)
            mCollider->changeCollision2DMini();
        else
            mCollider->changeCollisionMini();
        break;
    case 3:
        mCollider->changeCollisionSwim();
        break;
    case 4:
        mCollider->changeCollisionWallGrab(mActionFront);
        break;
    case 5:
        mCollider->changeCollisionGrabCeil(mActionFront);
        break;
    case 6:
        mCollider->changeCollisionPoleClimb(mActionFront);
        break;
    case 7:
        mCollider->changeCollisionHack(mHackKeeper->getCollisionPartsFilter());
        break;
    case 9:
        mCollider->changeCollisionRecovery(modelForm == 2);
        break;
    case 10:
        mCollider->changeCollisionAbyss(mActionFront);
        break;
    case 11:
        mCollider->changeCollisionNormal();
        break;
    }

    setCollisionShapeOffsetGround(0.0f);
    mIsFormDirty = false;
}

void PlayerFormSensorCollisionArranger::validateAttackSensor() {
    switch (mAttackSensorForm) {
    case 1:
        al::validateHitSensor(mPlayer, "SpinAttack");
        break;
    case 2:
        al::validateHitSensor(mPlayer, "TornadoAttack");
        break;
    }
}
