#include "Layout/CommonHorizontalList.h"

#include <cmath>
#include <prim/seadSafeString.h>

#include "Library/Base/StringUtil.h"
#include "Library/Controller/PadRumbleFunction.h"
#include "Library/Layout/LayoutActionFunction.h"
#include "Library/Layout/LayoutActor.h"
#include "Library/Layout/LayoutActorUtil.h"
#include "Library/Layout/LayoutInitInfo.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Se/SeFunction.h"

#include "Layout/ScrollBarParts.h"

namespace {
NERVE_IMPL(CommonHorizontalList, Active);
NERVE_IMPL(CommonHorizontalList, Deactive);
NERVE_IMPL(CommonHorizontalList, Decide);
NERVE_IMPL(CommonHorizontalList, DecideEnd);
NERVE_IMPL(CommonHorizontalList, Reject);
NERVE_IMPL(CommonHorizontalList, RejectEnd);

NERVES_MAKE_NOSTRUCT(CommonHorizontalList, Active, Deactive, Decide, DecideEnd, Reject, RejectEnd);
}  // namespace

// NON_MATCHING: register allocation differs (target saves firstTrans.x in s8/d8 callee-saved SIMD
// register across getPaneLocalTrans calls; target also re-reads mLayoutActor from struct rather
// than keeping it in a register, and has different instruction scheduling throughout)
CommonHorizontalList::CommonHorizontalList(al::LayoutActor* layoutActor,
                                           const al::LayoutInitInfo& initInfo, bool hasCursor)
    : al::NerveExecutor("汎用横リスト") {
    mLayoutActor = layoutActor;

    s32 paneChildNum = al::getPaneChildNum(layoutActor, "ListArea");

    for (s32 i = 0; i < paneChildNum; i++) {
        const char* childName = al::getPaneChildName(layoutActor, "ListArea", i);
        if (al::isEqualString(childName, "ParScroll")) {
            mScrollBarParts = new ScrollBarParts(layoutActor, initInfo, false);
            break;
        }
    }

    mItemPartCount = paneChildNum;
    if (mScrollBarParts)
        mItemPartCount = paneChildNum - 1;

    if (mItemPartCount >= 2) {
        mItemWidth = al::getPaneLocalTrans(layoutActor, "ParList00").x;

        mItemParts.allocBuffer(mItemPartCount, nullptr, 8);

        for (s32 i = 0; i < paneChildNum; i++) {
            const char* childName = al::getPaneChildName(layoutActor, "ListArea", i);
            if (!al::isEqualString(childName, "ParScroll")) {
                al::LayoutActor* parts = new al::LayoutActor("横リストパーツ");
                al::initLayoutPartsActor(parts, layoutActor, initInfo, childName, nullptr);
                auto* itemParts = reinterpret_cast<sead::PtrArray<al::LayoutActor>*>(&mItemParts);
                if (itemParts->size() < itemParts->capacity())
                    itemParts->pushBack(parts);
            }
        }

        auto* itemParts = reinterpret_cast<sead::PtrArray<al::LayoutActor>*>(&mItemParts);
        sead::Vector2f firstTrans = sead::Vector2f::zero;
        if (itemParts->size() > 0)
            firstTrans = al::getLocalTrans(itemParts->at(0));

        sead::Vector2f secondTrans = sead::Vector2f::zero;
        if (itemParts->size() >= 2)
            secondTrans = al::getLocalTrans(itemParts->at(1));

        f32 spacing = secondTrans.x - firstTrans.x;
        if (spacing <= 0.0f)
            spacing = -spacing;
        _0x54 = spacing;

        if (hasCursor) {
            mCursorActor = new al::LayoutActor("カーソル");
            al::initLayoutPartsActor(mCursorActor, layoutActor, initInfo, "ParCursor", nullptr);
            al::startAction(mCursorActor, "Wait");
        }

        initNerve(&Active, 0);

        mStringDataArray = new sead::WFixedSafeString<256>*[5];
        mStringLabels = new sead::FixedSafeString<128>[5];
        mDecideActionName = new sead::FixedSafeString<128>();
        mDecideActionName->format("%s", "Decide");

        s32 halfCount = mItemPartCount / -2;
        mScrollOffset = halfCount;
        mScrollOffsetPrev = halfCount;
        mSelectedIdx = 0;
    }
}

CommonHorizontalList::~CommonHorizontalList() = default;

// NON_MATCHING: wrong function size; sead::PtrArrayImpl reinterpret_cast bounds check generates
// extra branches vs original, and register allocation differs throughout
void CommonHorizontalList::initData(s32 itemCount) {
    mSelectedIdx = 0;
    mScrollOffset = mItemPartCount / -2;
    mScrollOffsetPrev = mScrollOffset;
    mStringDataCount = 0;
    mItemCount = itemCount;
    mEnableData = nullptr;
    mAnimTimer = 0;

    if (itemCount > 0) {
        auto* itemParts = reinterpret_cast<sead::PtrArray<al::LayoutActor>*>(&mItemParts);
        s32 count = mItemPartCount <= itemCount ? mItemPartCount : itemCount;
        for (s32 i = 0; i < count; i++)
            al::startAction(itemParts->at(i), "Wait");

        if (mScrollBarParts)
            mScrollBarParts->setupDataNum(mItemPartCount - 3, itemCount);

        mIsNewSelection = true;
    }
}

// NON_MATCHING: same as initData — wrong function size from PtrArrayImpl reinterpret_cast bounds
// check and register allocation differences
void CommonHorizontalList::initDataNoResetSelected(s32 itemCount) {
    mStringDataCount = 0;
    mItemCount = itemCount;
    mEnableData = nullptr;
    mAnimTimer = 0;

    if (itemCount > 0) {
        auto* itemParts = reinterpret_cast<sead::PtrArray<al::LayoutActor>*>(&mItemParts);
        s32 count = mItemPartCount <= itemCount ? mItemPartCount : itemCount;
        for (s32 i = 0; i < count; i++)
            al::startAction(itemParts->at(i), "Wait");

        if (mScrollBarParts)
            mScrollBarParts->setupDataNum(mItemPartCount - 3, itemCount);

        mIsNewSelection = true;
    }
}

// NON_MATCHING: same as initData — wrong function size from PtrArrayImpl reinterpret_cast bounds
// check and register allocation differences
void CommonHorizontalList::initDataWithIdx(s32 itemCount, s32 scrollOffset, s32 selectedIdx) {
    mSelectedIdx = selectedIdx;
    mScrollOffset = scrollOffset;
    mScrollOffsetPrev = 0;
    mStringDataCount = 0;
    mItemCount = itemCount;
    mEnableData = nullptr;
    mAnimTimer = 0;

    if (itemCount > 0) {
        auto* itemParts = reinterpret_cast<sead::PtrArray<al::LayoutActor>*>(&mItemParts);
        s32 count = mItemPartCount <= itemCount ? mItemPartCount : itemCount;
        for (s32 i = 0; i < count; i++)
            al::startAction(itemParts->at(i), "Wait");

        if (mScrollBarParts)
            mScrollBarParts->setupDataNum(mItemPartCount - 3, itemCount);

        mIsNewSelection = true;
    }
}

// NON_MATCHING: madd operand register swap (ldrsw result in x8 vs x9) — NonMatchingMinor
void CommonHorizontalList::addStringData(const sead::WFixedSafeString<256>* wstr,
                                         const char* paneName) {
    mStringDataArray[mStringDataCount] = const_cast<sead::WFixedSafeString<256>*>(wstr);
    auto* labels = reinterpret_cast<sead::FixedSafeString<128>*>(mStringLabels);
    labels[mStringDataCount].format("%s", paneName);
    mStringDataCount++;
}

void CommonHorizontalList::setGroupAnimData(const sead::FixedSafeString<64>* groupAnimNames,
                                            const char* partsName) {
    mGroupAnimNameArray = groupAnimNames;
    mGroupAnimPartsName = partsName;
}

void CommonHorizontalList::setImageData(nn::ui2d::TextureInfo** imageArray, const char* paneName) {
    mImageDataArray = imageArray;
    mImagePaneName = paneName;
}

void CommonHorizontalList::setSelectedIdx(s32 idx) {
    mSelectedIdx = idx;
    s32 offset = idx - mItemPartCount / 2;
    mScrollOffset = offset;
    mScrollOffsetPrev = offset;
}

void CommonHorizontalList::setEnableData(const bool* enableData) {
    mEnableData = enableData;
}

// NON_MATCHING: wrong function size; target saves x20 (passes mCursorPos addr directly vs stack)
void CommonHorizontalList::calcCursorPos(sead::Vector2f* outPos) const {
    *outPos = mCursorPos;
}

bool CommonHorizontalList::isActive() const {
    return al::isNerve(this, &Active);
}

bool CommonHorizontalList::isDeactive() const {
    return al::isNerve(this, &Deactive);
}

bool CommonHorizontalList::isDecideEnd() const {
    return al::isNerve(this, &DecideEnd);
}

bool CommonHorizontalList::isRejectEnd() const {
    return al::isNerve(this, &RejectEnd);
}

void CommonHorizontalList::update() {
    updateNerve();
}

// NON_MATCHING: wrong function size; register allocation and branch layout differ
void CommonHorizontalList::right() {
    if (mAnimTimer > 0)
        return;

    s32 prevSelectedIdx = mSelectedIdx;
    s32 prevScrollOffset = mScrollOffset;
    s32 maxIdx = mItemCount - 1;

    mSelectedIdx = prevSelectedIdx + 1;
    mScrollOffset = prevScrollOffset + 1;
    mScrollOffsetPrev = prevScrollOffset;

    al::IUseAudioKeeper* audioKeeper = nullptr;
    if (mLayoutActor)
        audioKeeper = mLayoutActor;

    const char* seName;
    if (prevSelectedIdx >= maxIdx) {
        mSelectedIdx = maxIdx;
        mScrollOffset = prevScrollOffset;
        seName = "ListEdge";
    } else {
        seName = "ListRight";
    }

    al::tryStartSe(audioKeeper, sead::SafeString(seName));

    if (mScrollOffsetPrev != mScrollOffset)
        mAnimTimer = mAnimTimerMax;
}

// NON_MATCHING: wrong function size; register allocation and branch layout differ
void CommonHorizontalList::left() {
    if (mAnimTimer > 0)
        return;

    s32 prevSelectedIdx = mSelectedIdx;
    s32 prevScrollOffset = mScrollOffset;

    mSelectedIdx = prevSelectedIdx - 1;
    mScrollOffset = prevScrollOffset - 1;
    mScrollOffsetPrev = prevScrollOffset;

    al::IUseAudioKeeper* audioKeeper = nullptr;
    if (mLayoutActor)
        audioKeeper = mLayoutActor;

    const char* seName;
    if (prevSelectedIdx <= 0) {
        mSelectedIdx = 0;
        mScrollOffset = prevScrollOffset;
        seName = "ListEdge";
    } else {
        seName = "ListLeft";
    }

    al::tryStartSe(audioKeeper, sead::SafeString(seName));

    if (mScrollOffsetPrev != mScrollOffset)
        mAnimTimer = mAnimTimerMax;
}

// NON_MATCHING: wrong function size; register allocation and branch layout differ
void CommonHorizontalList::pageUp() {
    if (!al::isNerve(this, &Active))
        return;

    if (mAnimTimer > 0)
        return;

    s32 scrollOffset = mScrollOffset;
    if (mAnimTimer == 0 && scrollOffset == 0)
        return;

    s32 step = mItemPartCount - 4;
    s32 newSelectedIdx = mSelectedIdx - step;
    mSelectedIdx = newSelectedIdx;
    mScrollOffsetPrev = scrollOffset;
    mScrollOffset = scrollOffset - step;

    al::IUseAudioKeeper* audioKeeper = nullptr;
    if (mLayoutActor)
        audioKeeper = mLayoutActor;

    const char* seName;
    if (newSelectedIdx < 0) {
        mSelectedIdx = 0;
        mScrollOffset = mItemPartCount / -2;
        seName = "ListEdge";
    } else {
        seName = "ListLeft";
    }

    al::tryStartSe(audioKeeper, sead::SafeString(seName));

    if (mScrollOffsetPrev != mScrollOffset)
        mAnimTimer = mAnimTimerMax;
}

// NON_MATCHING: wrong function size; register allocation and branch layout differ
void CommonHorizontalList::pageDown() {
    if (!al::isNerve(this, &Active))
        return;

    if (mAnimTimer > 0)
        return;

    s32 scrollOffset = mScrollOffset;
    s32 itemCount = mItemCount;
    if (scrollOffset == itemCount + 2 - mItemPartCount)
        return;

    mScrollOffsetPrev = scrollOffset;

    al::IUseAudioKeeper* audioKeeper = nullptr;
    if (mLayoutActor)
        audioKeeper = mLayoutActor;

    const char* seName = nullptr;
    if (itemCount >= mItemPartCount - 2) {
        s32 newSelectedIdx = mItemPartCount + mSelectedIdx - 4;
        s32 newScrollOffset = mItemPartCount + scrollOffset - 4;
        mSelectedIdx = newSelectedIdx;
        mScrollOffset = newScrollOffset;

        if (newSelectedIdx >= itemCount) {
            s32 last = itemCount - 1;
            mSelectedIdx = last;
            mScrollOffset = last - mItemPartCount / 2;
            seName = "ListEdge";
        } else {
            seName = "ListRight";
        }

        al::tryStartSe(audioKeeper, sead::SafeString(seName));

        s32 newScroll = mScrollOffset;
        s32 prevScroll = mScrollOffsetPrev;
        if (prevScroll == newScroll)
            newScroll = mScrollOffsetPrev;
        else
            mAnimTimer = mAnimTimerMax;
    } else {
        s32 newSelectedIdx = itemCount - 1;
        s32 newScrollOffset = newSelectedIdx - mItemPartCount / 2;
        mSelectedIdx = newSelectedIdx;
        mScrollOffset = newScrollOffset;
    }

    if (mScrollOffsetPrev != mScrollOffset)
        mAnimTimer = mAnimTimerMax;
}

// NON_MATCHING: wrong function size; register allocation and branch layout differ
void CommonHorizontalList::jumpLeft() {
    if (!al::isNerve(this, &Active))
        return;

    mSelectedIdx = 0;
    s32 prevScrollOffset = mScrollOffset;
    s32 halfCount = mItemPartCount / 2;
    mScrollOffset = mItemPartCount / -2;
    mScrollOffsetPrev = prevScrollOffset;

    al::IUseAudioKeeper* audioKeeper = nullptr;
    if (mLayoutActor)
        audioKeeper = mLayoutActor;

    const char* seName;
    if (prevScrollOffset + halfCount != 0) {
        mAnimTimer = mAnimTimerMax;
        seName = "ListJump";
    } else {
        seName = "ListEdge";
    }

    al::tryStartSe(audioKeeper, sead::SafeString(seName));
}

void CommonHorizontalList::jumpRight() {
    if (!al::isNerve(this, &Active))
        return;

    s32 itemCount = mItemCount;
    s32 lastIdx = itemCount - 1;
    mSelectedIdx = lastIdx;
    s32 prevScrollOffset = mScrollOffset;
    s32 newScrollOffset = lastIdx - mItemPartCount / 2;
    mScrollOffset = newScrollOffset;
    mScrollOffsetPrev = prevScrollOffset;

    al::IUseAudioKeeper* audioKeeper = nullptr;
    if (mLayoutActor)
        audioKeeper = mLayoutActor;

    const char* seName;
    if (prevScrollOffset == newScrollOffset) {
        seName = "ListEdge";
    } else {
        mAnimTimer = mAnimTimerMax;
        seName = "ListJump";
    }

    al::tryStartSe(audioKeeper, sead::SafeString(seName));
}

void CommonHorizontalList::decide(const char* decideLabel) {
    if (!al::isNerve(this, &Active))
        return;

    if (mAnimTimer >= 1) {
        mAnimTimer = 0;
        updateParts();
    }

    mDecideActionName->format("%s", decideLabel);
    al::setNerve(this, &Decide);
}

// NON_MATCHING: wrong function size; PtrArrayImpl reinterpret_cast bounds checks, register
// allocation, and branch layout all differ throughout
void CommonHorizontalList::updateParts() {
    if (mScrollBarParts)
        mScrollBarParts->updateIdx(mSelectedIdx);

    s32 curScrollOffset = mScrollOffset;
    s32 curScrollPrev = mScrollOffsetPrev;

    f32 t = al::easeOut((f32)(mAnimTimer - 2) / (f32)(mAnimTimerMax - 2));
    t = al::lerpValue(0.0f, 1.0f, t);
    f32 blendOffset = t * _0x54;
    if (curScrollPrev >= curScrollOffset)
        blendOffset = -blendOffset;

    auto* itemParts = reinterpret_cast<sead::PtrArray<al::LayoutActor>*>(&mItemParts);
    s32 visibleCount = mItemPartCount;
    if (visibleCount >= 1) {
        f32 basePos = mItemWidth + blendOffset;
        f32 spacing = _0x54;

        f32 offset = 0.0f;
        for (s32 i = 0; i < visibleCount; i++) {
            s32 dataIdx = curScrollOffset + i;

            al::LayoutActor* partActor =
                (u32)i < (u32)itemParts->capacity() ? itemParts->at(i) : nullptr;

            sead::Vector2f localTrans = al::getLocalTrans(partActor);
            sead::Vector2f newTrans(basePos - offset, localTrans.y);
            al::setLocalTrans(partActor, newTrans);

            if (dataIdx < 0 || dataIdx >= mItemCount) {
                if (partActor)
                    al::startAction(partActor, "Hide");
            } else {
                if (mEnableData) {
                    if (partActor)
                        al::startAction(partActor, mEnableData[dataIdx] ? "On" : "Off", "OnOff");
                }

                if (mGroupAnimNameArray) {
                    const sead::FixedSafeString<64>& groupAnim = mGroupAnimNameArray[dataIdx];
                    if (partActor)
                        al::startAction(partActor, groupAnim.cstr(), mGroupAnimPartsName);
                }

                auto* labels = reinterpret_cast<sead::FixedSafeString<128>*>(mStringLabels);
                for (s32 j = 0; j < mStringDataCount; j++) {
                    al::setPaneString(partActor, labels[j].cstr(),
                                      mStringDataArray[j]->getStringTop());
                }

                if (mImageDataArray && mImageDataArray[dataIdx])
                    al::setPaneTexture(partActor, mImagePaneName, mImageDataArray[dataIdx]);

                if (mCursorVisualActor) {
                    if (dataIdx == mSelectedIdx) {
                        if (!al::isNerve(this, &Deactive)) {
                            if (mCursorVisualActor)
                                al::startAction(mCursorVisualActor,
                                                mEnableData && !mEnableData[dataIdx] ? "Off" : "On",
                                                "OnOff");

                            if (al::isNear(t, 1.0f, 0.001f)) {
                                if (mCursorVisualActor)
                                    al::tryStartAction(mCursorVisualActor, "Select");
                                al::getActionFrameMax(mCursorVisualActor, "Select");
                                al::setActionFrame(mCursorVisualActor, 1.0f);
                            }

                            if (mIsNewSelection) {
                                if (mCursorVisualActor)
                                    al::tryStartAction(mCursorVisualActor, "Select");
                                if (mCursorVisualActor) {
                                    f32 maxFrame =
                                        al::getActionFrameMax(mCursorVisualActor, "Select");
                                    al::setActionFrame(mCursorVisualActor, maxFrame);
                                }
                            }
                        }
                    }
                }

                if (al::isNearZero(sead::Vector2f(t, 0.0f), 0.001f) && dataIdx == mSelectedIdx &&
                    !al::isNerve(this, &Deactive)) {
                    if (partActor && !al::isActionPlaying(partActor, "Select")) {
                        al::startAction(partActor, "Select");
                        if (mIsNewSelection) {
                            f32 maxFrame = al::getActionFrameMax(partActor, "Select");
                            al::setActionFrame(partActor, maxFrame);
                            mIsNewSelection = false;
                        } else {
                            f32 pct = 0.5f;
                            if (mItemCount >= 2)
                                pct = (f32)mSelectedIdx / (f32)(mItemCount - 1);
                            f32 angle = pct * 90.0f * sead::Mathf::pi2() / 360.0f;
                            al::PadRumbleParam param;
                            param.volumeLeft = mRumbleStrength * cosf(angle);
                            param.volumeRight = mRumbleStrength * sinf(angle);
                            alPadRumbleFunction::startPadRumbleNo3DWithParam(
                                alPadRumbleFunction::getPadRumbleDirector(mLayoutActor),
                                "コッ（中）", param);
                        }
                    }
                    offset += spacing;
                    continue;
                }
                if (partActor)
                    al::startAction(partActor, "Wait");
            }
            offset += spacing;
        }
    }

    if (mAnimTimer < 0) {
        s32 diff = mSelectedIdx - mScrollOffset;
        al::LayoutActor* cursorTarget =
            (u32)diff < (u32)itemParts->capacity() ? itemParts->at(diff) : nullptr;
        sead::Vector2f panePos;
        al::calcPaneTrans(&panePos, cursorTarget, "Cursor");
        mCursorPos = panePos;
        if (mCursorActor)
            al::setLocalTrans(mCursorActor, mCursorPos);
    }

    if (mAnimTimer >= 0)
        mAnimTimer--;

    al::updateLayoutPaneRecursive(mLayoutActor);
}

void CommonHorizontalList::reject() {
    if (!al::isNerve(this, &Active))
        return;

    if (mAnimTimer >= 1) {
        mAnimTimer = 0;
        updateParts();
    }

    al::setNerve(this, &Reject);
}

void CommonHorizontalList::deactivate() {
    if (!al::isNerve(this, &Active))
        return;
    al::setNerve(this, &Deactive);
}

void CommonHorizontalList::activate() {
    if (al::isNerve(this, &Active))
        return;
    if (al::isNerve(this, &Decide))
        return;
    al::setNerve(this, &Active);
}

void CommonHorizontalList::exeActive() {
    updateParts();
}

void CommonHorizontalList::exeDeactive() {
    updateParts();
}

void CommonHorizontalList::exeDecide() {
    s32 cursorOff = mSelectedIdx - mScrollOffset;
    auto* itemParts = reinterpret_cast<sead::PtrArray<al::LayoutActor>*>(&mItemParts);

    if (al::isFirstStep(this)) {
        al::LayoutActor* parts = itemParts->at(cursorOff);
        al::startAction(parts, mDecideActionName->cstr());
        mIsNewSelection = true;
    }

    al::LayoutActor* parts = itemParts->at(cursorOff);
    if (al::isActionEnd(parts))
        al::setNerve(this, &DecideEnd);
}

void CommonHorizontalList::exeDecideEnd() {}

void CommonHorizontalList::exeReject() {
    s32 cursorOff = mSelectedIdx - mScrollOffset;
    auto* itemParts = reinterpret_cast<sead::PtrArray<al::LayoutActor>*>(&mItemParts);

    if (al::isFirstStep(this)) {
        al::LayoutActor* parts = itemParts->at(cursorOff);
        al::startAction(parts, "Reject");
        mIsNewSelection = true;
    }

    al::LayoutActor* parts = itemParts->at(cursorOff);
    if (al::isActionEnd(parts))
        al::setNerve(this, &RejectEnd);
}

void CommonHorizontalList::exeRejectEnd() {
    al::setNerve(this, &Active);
}

// NON_MATCHING: wrong function size; target saves x20 and passes mCursorPos addr directly,
// uses different bounds check for mItemParts, and stores mCursorPos differently
void CommonHorizontalList::updateCursorPos() {
    s32 cursorOff = mSelectedIdx - mScrollOffset;
    auto* itemParts = reinterpret_cast<sead::PtrArray<al::LayoutActor>*>(&mItemParts);
    al::LayoutActor* cursorActor =
        (u32)cursorOff < (u32)itemParts->capacity() ? itemParts->at(cursorOff) : nullptr;

    sead::Vector2f panePos;
    al::calcPaneTrans(&panePos, cursorActor, "Cursor");
    mCursorPos = panePos;
    if (mCursorActor)
        al::setLocalTrans(mCursorActor, mCursorPos);
}
