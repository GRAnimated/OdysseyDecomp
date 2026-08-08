#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class GamePadSystem;
}

class PlayerInitInfo {
public:
    al::GamePadSystem* gamePadSystem;
    sead::Matrix34f* viewMtx;
    u32 portNo;
    u32 padding_14;
    const char* modelName;
    const char* capTypeName;
    sead::Vector3f trans;
    sead::Quatf quat;
    bool isNeedCreateNoseNeedle;
    bool isClosetScenePlayer;
    u8 padding_46[2];
};

