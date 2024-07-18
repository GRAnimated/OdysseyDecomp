#include "Util/ItemGenerator.h"
#include "Enemy/KuriboMini.h"
#include "Library/Item/ItemUtil.h"
#include "Library/LiveActor/ActorInitInfo.h"
#include "Library/LiveActor/ActorPoseKeeper.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Math/MathAngleUtil.h"
#include "Library/Math/MathLengthUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Math/VectorUtil.h"
#include "Util/ItemUtil.h"
#include "math/seadVectorFwd.h"

ItemGenerator::ItemGenerator() {}

ItemGenerator::ItemGenerator(al::LiveActor* creator, const al::ActorInitInfo& info)
    : mCreator(creator) {
    initLinkShine(creator, info, false);
}

void ItemGenerator::initLinkShine(al::LiveActor* creator, const al::ActorInitInfo& info, bool a3) {
    mCreator = creator;
    mItemType = rs::getItemType(info);

    if (isShine()) {
        mLinkShine = rs::tryInitLinkShine(info, "ShineActor", 0);
    } else if (rs::isItemTypeKuriboMini(&mKuriboMiniCount, mItemType)) {
        mKuriboMiniArray = new KuriboMini*[mKuriboMiniCount];
        for (s32 i = 0; i < mKuriboMiniCount; i++) {
            mKuriboMiniArray[i] = new KuriboMini("マメクリボー[アイテム]");
            al::initCreateActorNoPlacementInfo(mKuriboMiniArray[i], info);
            mKuriboMiniArray[i]->makeActorDead();
        }
    } else {
        rs::tryInitItemByPlacementInfo(mCreator, info, a3);
    }
}

void ItemGenerator::initNoLinkShine(al::LiveActor* creator, const al::ActorInitInfo& info,
                                    bool a3) {
    mCreator = creator;
    mItemType = rs::getItemType(info);

    if (isShine()) {
        mLinkShine = rs::initShineByPlacementInfo(info);
    } else if (rs::isItemTypeKuriboMini(&mKuriboMiniCount, mItemType)) {
        mKuriboMiniArray = new KuriboMini*[mKuriboMiniCount];
        for (s32 i = 0; i < mKuriboMiniCount; i++) {
            mKuriboMiniArray[i] = new KuriboMini("マメクリボー[アイテム]");
            al::initCreateActorNoPlacementInfo(mKuriboMiniArray[i], info);
            mKuriboMiniArray[i]->makeActorDead();
        }
    } else {
        rs::tryInitItemByPlacementInfo(mCreator, info, a3);
    }
}

void ItemGenerator::initHintPhotoShine(al::LiveActor* creator, const al::ActorInitInfo& info) {
    mCreator = creator;
    mItemType = rs::ItemType::Shine;
    mLinkShine = rs::tryInitLinkShineHintPhoto(info, "ShineActor", 0);
}

void ItemGenerator::createShineEffectInsideObject(const al::ActorInitInfo& info) {
    rs::createShineEffectInsideObject(mLinkShine, mCreator, info);
}

bool ItemGenerator::tryUpdateHintTransIfExistShine() {
    const sead::Vector3f& trans = al::getTrans(mCreator);
    return tryUpdateHintTransIfExistShine(trans);
}

bool ItemGenerator::tryUpdateHintTransIfExistShine(const sead::Vector3f& trans) {
    if (mLinkShine && field_20 <= 0) {
        rs::updateHintTrans(mLinkShine, trans);
        return true;
    }
    return false;
}

void generateKuribos(KuriboMini** kuriboMiniArray, s32 kuriboMiniCount, al::LiveActor* creator,
                     const sead::Vector3f& vec) {
    sead::Vector3f v = sead::Vector3f(0.0f, 1.0f, 0.0f);
    al::verticalizeVec(&v, vec, v);

    if (!al::tryNormalizeOrZero(&v))
        al::calcDirVerticalAny(&v, vec);

    if (kuriboMiniCount < 1)
        return;

    for (s32 i = 0; i < kuriboMiniCount; i++) {
        sead::Vector3f unk = vec;
        al::rotateVectorDegree(&unk, unk, v, al::normalize(i, 0, kuriboMiniCount) * 360.0f);
        al::normalize(&unk);

        sead::Quatf quat = sead::Quatf::unit;
        al::makeQuatFrontUp(&quat, unk, v);
        al::updatePoseQuat(kuriboMiniArray[i], quat);

        f32 v13 = ((quat.y * 150.0f) - (quat.z * 0.0f)) + (quat.w * 0.0f);
        f32 v14 = (quat.w * 0.0f) + ((quat.z * 0.0f) - (quat.x * 150.0f));
        f32 v15 = (quat.w * 150.0f) + ((quat.x * 0.0f) - (quat.y * 0.0f));
        f32 v16 = (-(quat.x * 0.0f) - (quat.y * 0.0f)) - (quat.z * 150.0f);
        sead::Vector3f pos;
        pos.x = ((quat.y * v15) + ((quat.w * v13) - (quat.z * v14))) - (quat.x * v16);
        pos.y = (((quat.z * v13) + (quat.w * v14)) - (quat.x * v15)) - (quat.y * v16);
        pos.z = ((quat.w * v15) + ((quat.x * v14) - (quat.y * v13))) - (quat.z * v16);

        al::setTrans(kuriboMiniArray[i], al::getTrans(creator) - pos);
        kuriboMiniArray[i]->appearPopBack();
    }
}

bool ItemGenerator::isEnableGenerateByCount(s32 count) const {
    return field_20 < count;
}

void ItemGenerator::generate(const sead::Vector3f& pos, const sead::Quatf& quat) {
    if (mItemType != -1) {
        if (mLinkShine)
            rs::appearPopupShine(mLinkShine, pos);
        else {
            // TODO: probably an inlined function here
            KuriboMini** kuriboMiniArray = mKuriboMiniArray;
            al::LiveActor* creator = mCreator;
            if (kuriboMiniArray) {
                s32 kuriboMiniCount = mKuriboMiniCount;
                sead::Vector3f vec = sead::Vector3f(0.0f, 0.0f, 0.0f);
                al::calcQuatFront(&vec, quat);
                generateKuribos(kuriboMiniArray, kuriboMiniCount, creator, vec);
            } else {
                al::appearItem(mCreator, pos, quat, nullptr);
            }
        }
        field_20++;
    }
}

void ItemGenerator::generate(const sead::Vector3f& pos, const sead::Vector3f& vec) {
    if (mItemType != -1) {
        if (mLinkShine)
            rs::appearPopupShine(mLinkShine, pos);
        else {
            // TODO: probably an inlined function here
            KuriboMini** kuriboMiniArray = mKuriboMiniArray;
            al::LiveActor* creator = mCreator;
            if (kuriboMiniArray) {
                s32 kuriboMiniCount = mKuriboMiniCount;
                generateKuribos(kuriboMiniArray, kuriboMiniCount, creator, vec);
            } else {
                al::appearItem(mCreator, pos, vec, nullptr);
            }
        }
        field_20++;
    }
}

bool ItemGenerator::tryGenerate(const sead::Vector3f& pos, const sead::Quatf& quat, s32 count) {
    if (mItemType != -1 && isEnableGenerateByCount(count)) {
        generate(pos, quat);
        return true;
    }
    return false;
}

bool ItemGenerator::tryGenerate(const sead::Vector3f& pos, const sead::Vector3f& vec, s32 count) {
    if (mItemType != -1 && isEnableGenerateByCount(count)) {
        generate(pos, vec);
        return true;
    }
    return false;
}

bool ItemGenerator::isNone() const {
    return mItemType == -1;
}

bool ItemGenerator::isShine() const {
    return mItemType == rs::ItemType::Shine;
}

bool ItemGenerator::isLifeUp() const {
    return mItemType == rs::ItemType::LifeUpItem || mItemType == rs::ItemType::LifeMaxUpItem;
}

bool ItemGenerator::isLifeMaxUp() const {
    return mItemType == rs::ItemType::LifeMaxUpItem;
}

bool ItemGenerator::isCoin() const {
    return mItemType == rs::ItemType::Coin;
}

bool ItemGenerator::isCoinBlow() const {
    return mItemType == rs::ItemType::CoinBlow;
}

bool ItemGenerator::isCoinStackBound() const {
    return mItemType == rs::ItemType::CoinStackBound;
}

bool ItemGenerator::isKuriboMini3() const {
    return mItemType == rs::ItemType::KuriboMini3;
}

bool ItemGenerator::isKuriboMini8() const {
    return mItemType == rs::ItemType::KuriboMini8;
}
