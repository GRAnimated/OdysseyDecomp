#include "Npc/PeachOnKoopaAnimInfo.h"

#include "Library/Base/StringUtil.h"

PeachOnKoopaAnimInfo::PeachOnKoopaAnimInfo() = default;

void PeachOnKoopaAnimInfo::update(const char* body, const char* leftHand,
                                  const char* rightHand) {
    mIsBodyChanged = false;
    mIsLeftHandChanged = false;
    mIsRightHandChanged = false;

    if ((body == nullptr) != (mBodyAnim == nullptr) ||
        (body && mBodyAnim && !al::isEqualString(body, mBodyAnim))) {
        mIsBodyChanged = true;
        mBodyAnim = body;
    }

    if ((leftHand == nullptr) != (mLeftHandAnim == nullptr) ||
        (leftHand && mLeftHandAnim && !al::isEqualString(leftHand, mLeftHandAnim))) {
        mIsLeftHandChanged = true;
        mLeftHandAnim = leftHand;
    }

    if ((rightHand == nullptr) != (mRightHandAnim == nullptr) ||
        (rightHand && mRightHandAnim && !al::isEqualString(rightHand, mRightHandAnim))) {
        mIsRightHandChanged = true;
        mRightHandAnim = rightHand;
    }
}
