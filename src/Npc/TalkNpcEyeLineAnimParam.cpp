#include "Npc/TalkNpcEyeLineAnimParam.h"

#include "Library/Yaml/ByamlIter.h"
#include "Library/Yaml/ByamlUtil.h"

TalkNpcEyeLineAnimParam::TalkNpcEyeLineAnimParam()
    : mIsValid(false), mBaseJointName(nullptr), mMinDegreeH(20.0f), mMaxDegreeH(60.0f),
      mMinDegreeV(20.0f), mMaxDegreeV(60.0f), mLocalAxisFront(0.0f, 0.0f, 1.0f), mOffsetY(0.0f) {}

void TalkNpcEyeLineAnimParam::init(const al::ByamlIter& iter) {
    al::ByamlIter eyeLineIter;
    if (!al::tryGetByamlIterByKey(&eyeLineIter, iter, "EyeLineAnimCtrlParam"))
        return;

    mIsValid = true;
    mBaseJointName = al::getByamlKeyString(eyeLineIter, "BaseJointName");
    al::tryGetByamlF32(&mMinDegreeH, eyeLineIter, "MinDegreeH");
    al::tryGetByamlF32(&mMaxDegreeH, eyeLineIter, "MaxDegreeH");
    al::tryGetByamlF32(&mMinDegreeV, eyeLineIter, "MinDegreeV");
    al::tryGetByamlF32(&mMaxDegreeV, eyeLineIter, "MaxDegreeV");
    al::tryGetByamlF32(&mOffsetY, eyeLineIter, "OffsetY");
    al::tryGetByamlV3f(&mLocalAxisFront, eyeLineIter, "LocalAxisFront");
}

const char* TalkNpcEyeLineAnimParam::getEyeMoveAnimName() {
    return "EyeMove";
}

const char* TalkNpcEyeLineAnimParam::getEyeResetAnimName() {
    return "EyeReset";
}
