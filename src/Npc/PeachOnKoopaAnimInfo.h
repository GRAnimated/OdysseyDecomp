#pragma once

class PeachOnKoopaAnimInfo {
public:
    PeachOnKoopaAnimInfo();

    void update(const char* body, const char* leftHand, const char* rightHand);

    bool mIsBodyChanged = false;
    bool mIsLeftHandChanged = false;
    bool mIsRightHandChanged = false;
    const char* mBodyAnim = nullptr;
    const char* mLeftHandAnim = nullptr;
    const char* mRightHandAnim = nullptr;
};
