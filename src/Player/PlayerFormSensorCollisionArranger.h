#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
}
class IPlayerModelChanger;
class PlayerColliderHakoniwa;
class PlayerHackKeeper;

class PlayerFormSensorCollisionArranger {
public:
    PlayerFormSensorCollisionArranger(al::LiveActor* player, PlayerColliderHakoniwa* collider,
                                      const IPlayerModelChanger* modelChanger,
                                      const PlayerHackKeeper* hackKeeper);

    void setFormModel3D();
    void setFormModel2D();
    void setFormActionSquat();
    void setFormActionStandup();
    void setFormActionWallGrab(const sead::Vector3f& front);
    void setFormActionGrabCeil(const sead::Vector3f& front);
    void setFormActionPoleClimb(const sead::Vector3f& front);
    void setFormActionSwim();
    void setFormActionHack();
    void setFormActionBind(bool isBind);
    void setFormActionRecovery();
    void setFormActionAbyss(const sead::Vector3f& front);
    void setFormActionDead();
    void setFormAttackSensorNone();
    void setFormAttackSensorSpin();
    void setFormAttackSensorTornado();
    void setCollisionShapeOffsetGround(f32 offset);
    const char* getHeadSensorName() const;
    const sead::Vector3f& getHeadPos() const;
    f32 getHeadRadius() const;
    const sead::Vector3f& getBodyPos() const;
    bool isEnableSafetyPointForm() const;
    void update();
    void syncForm();
    void validateAttackSensor();

private:
    al::LiveActor* mPlayer;
    PlayerColliderHakoniwa* mCollider;
    const IPlayerModelChanger* mModelChanger;
    const PlayerHackKeeper* mHackKeeper;
    s32 mModelForm = 0;
    s32 mActionForm = 0;
    s32 mAttackSensorForm = 0;
    bool mIsFormDirty = false;
    u8 _2d[3];
    sead::Vector3f mActionFront = {0.0f, 0.0f, 0.0f};
    bool mIsBind = false;
    u8 _3d[3];
};

static_assert(sizeof(PlayerFormSensorCollisionArranger) == 0x40);
