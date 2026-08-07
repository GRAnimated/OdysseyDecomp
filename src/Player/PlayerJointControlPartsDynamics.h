#pragma once

#include <basis/seadTypes.h>

namespace al {
class JointSpringController;
class JointSpringControllerHolder;
class JointSpringTransController;
class LiveActor;
}
namespace sead {
template <typename T>
class Matrix34;
using Matrix34f = Matrix34<f32>;
}
class PlayerConst;
class PlayerJointControlCostumeAdjust;

class PlayerJointControlPartsDynamics {
public:
    PlayerJointControlPartsDynamics(const al::LiveActor*, const PlayerConst*, bool, bool, bool,
                                    s32);

    void createFollowJawJointController(const al::LiveActor*, const sead::Matrix34f*);
    void update(f32, f32);
    void invalidateCapDynamics();
    void resetDynamics();

private:
    PlayerJointControlCostumeAdjust* mCostumeAdjust;
    bool _8;
    u8 _9[7];
    const PlayerConst* mConst;
    al::JointSpringControllerHolder* _18;
    al::JointSpringController* _20;
    al::JointSpringControllerHolder* _28;
    al::JointSpringController* _30;
    al::JointSpringController* _38;
    al::JointSpringTransController* _40;
    al::JointSpringTransController* _48;
    al::JointSpringTransController* _50;
    f32 _58;
    f32 _5c;
};

static_assert(sizeof(PlayerJointControlPartsDynamics) == 0x60);
