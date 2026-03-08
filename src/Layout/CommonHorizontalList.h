#pragma once

#include <container/seadPtrArray.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

#include "Library/Nerve/NerveExecutor.h"

namespace al {
class LayoutActor;
class LayoutInitInfo;
}  // namespace al

namespace nn::ui2d {
class TextureInfo;
}

class ScrollBarParts;

class CommonHorizontalList : public al::NerveExecutor {
public:
    CommonHorizontalList(al::LayoutActor* layoutActor, const al::LayoutInitInfo& initInfo,
                         bool hasCursor);
    ~CommonHorizontalList();

    void initData(s32 itemCount);
    void initDataNoResetSelected(s32 itemCount);
    void initDataWithIdx(s32 itemCount, s32 scrollOffset, s32 selectedIdx);
    void addStringData(const sead::WFixedSafeString<256>* wstr, const char* paneName);
    void setGroupAnimData(const sead::FixedSafeString<64>* groupAnimNames, const char* partsName);
    void setImageData(nn::ui2d::TextureInfo** imageArray, const char* paneName);
    void setSelectedIdx(s32 idx);
    void setEnableData(const bool* enableData);
    void calcCursorPos(sead::Vector2f* outPos) const;
    bool isActive() const;
    bool isDeactive() const;
    bool isDecideEnd() const;
    bool isRejectEnd() const;

    void update();
    void right();
    void left();
    void pageUp();
    void pageDown();
    void jumpLeft();
    void jumpRight();
    void decide(const char* decideLabel);
    void updateParts();
    void reject();
    void deactivate();
    void activate();

    void exeActive();
    void exeDeactive();
    void exeDecide();
    void exeDecideEnd();
    void exeReject();
    void exeRejectEnd();

    void updateCursorPos();

    s32 getSelectedIdx() const { return mSelectedIdx; }

    s32 getItemCount() const { return mItemCount; }

private:
    al::LayoutActor* mLayoutActor = nullptr;
    sead::PtrArrayImpl mItemParts;
    ScrollBarParts* mScrollBarParts = nullptr;
    al::LayoutActor* mCursorActor = nullptr;
    s32 mItemPartCount = -1;
    s32 mSelectedIdx = -1;
    s32 mScrollOffset = -1;
    s32 mScrollOffsetPrev = -1;
    f32 mItemWidth = -1.0f;
    s32 mAnimTimer = -1;
    s32 mAnimTimerMax = 5;
    f32 _0x54 = 0.0f;
    sead::Vector2f mCursorPos = sead::Vector2f::zero;
    sead::WFixedSafeString<256>** mStringDataArray = nullptr;
    sead::BufferedSafeStringBase<char>* mStringLabels = nullptr;
    nn::ui2d::TextureInfo** mImageDataArray = nullptr;
    const bool* mEnableData = nullptr;
    const sead::FixedSafeString<64>* mGroupAnimNameArray = nullptr;
    s32 mStringDataCount = -1;
    s32 mItemCount = -1;
    const char* mImagePaneName = nullptr;
    const char* mGroupAnimPartsName = nullptr;
    al::LayoutActor* mCursorVisualActor = nullptr;
    bool mIsNewSelection = false;
    sead::BufferedSafeStringBase<char>* mDecideActionName = nullptr;
    f32 mRumbleStrength = 0.3f;
};

static_assert(sizeof(CommonHorizontalList) == 0xC0);
