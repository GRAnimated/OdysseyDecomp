#include "Player/PlayerCounterIceWater.h"

#include "Library/Effect/EffectSystemInfo.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/LiveActor.h"
#include "Library/Obj/EffectObjFunction.h"
#include "Library/Se/SeFunction.h"

#include "MapObj/CapMessageShowInfo.h"
#include "Player/PlayerConst.h"
#include "Util/JudgeUtil.h"
#include "Util/PlayerCollisionUtil.h"

constexpr s32 IceWaterHighLevelFrames = 90;
constexpr s32 IceWaterMediumLevelFrames = 180;

PlayerCounterIceWater::PlayerCounterIceWater(al::LiveActor* player,
                                             const al::ActorInitInfo& initInfo,
                                             const PlayerConst* playerConst,
                                             const IUsePlayerCollision* collider, IJudge* judge)
    : mPlayer(player), mConst(playerConst), mCollider(collider), mJudge(judge) {
    mIsShowCapMsg = !rs::isShowCapMsgPlayerInIceWaterFirst(player);
    mIceEffect = new al::LiveActor("氷水エフェクト");
    al::EffectObjFunction::initActorEffectObjNoArchive(mIceEffect, initInfo, "ScreenColdWater");
    al::invalidateClipping(mIceEffect);
    mIceEffect->makeActorAlive();
}

inline void PlayerCounterIceWater::deleteIceEffect(al::LiveActor* iceEffect, IceWaterLevel level) {
    if (level != IceWaterLevel::High) {
        if (level != IceWaterLevel::Medium) {
            if (level == IceWaterLevel::Low)
                al::tryDeleteEffect(iceEffect, "EnterColdWaterLv1");
        } else {
            al::tryDeleteEffect(iceEffect, "EnterColdWaterLv2");
        }
    } else {
        al::tryDeleteEffect(iceEffect, "EnterColdWaterLv3");
    }
}

void PlayerCounterIceWater::clearIceWaterCount() {
    deleteIceEffect(mIceEffect, mIceWaterLevel);
    mIceWaterCount = 0;
    mRecoveryCount = 0;
    mIceWaterLevel = IceWaterLevel::None;
    mIsInIceWater = false;
}

void PlayerCounterIceWater::updateCount(bool isInIceWater, bool isOnGround) {
    bool isJudge = rs::updateJudgeAndResult(mJudge);
    IceWaterLevel previousLevel = mIceWaterLevel;
    mIsInIceWater = isJudge;

    if (isJudge && isInIceWater) {
        s32 iceWaterCount = mIceWaterCount;
        if (iceWaterCount == 0) {
            al::startHitReaction(mPlayer, "氷水入水");
            if (mIsShowCapMsg)
                rs::tryShowCapMsgPlayerInIceWaterFirst(mPlayer);
            iceWaterCount = mIceWaterCount;
        }

        if (isOnGround) {
            if (iceWaterCount == 0)
                mIceWaterCount = 1;
            else if (isTriggerDamage())
                mIceWaterCount++;
        } else {
            mIceWaterCount = iceWaterCount + 1;
        }

        s32 remaining = mConst->getIceWaterDamageInterval() - mIceWaterCount;
        if (remaining <= IceWaterHighLevelFrames)
            mIceWaterLevel = IceWaterLevel::High;
        else if (remaining <= IceWaterMediumLevelFrames)
            mIceWaterLevel = IceWaterLevel::Medium;
        else
            mIceWaterLevel = IceWaterLevel::Low;
        mRecoveryCount = 0;
    } else {
        if (isOnGround && isTriggerDamage())
            mIceWaterCount++;
        updateRecoveryCountImpl();
    }

    if (previousLevel != mIceWaterLevel) {
        al::LiveActor* iceEffect = mIceEffect;
        IceWaterLevel previousEffectLevel =
            previousLevel < IceWaterLevel::Low ? IceWaterLevel::Low : previousLevel;
        deleteIceEffect(iceEffect, previousEffectLevel);

        iceEffect = mIceEffect;
        IceWaterLevel currentEffectLevel = mIceWaterLevel;
        bool isEmitting = false;
        if (currentEffectLevel != IceWaterLevel::High) {
            if (currentEffectLevel != IceWaterLevel::Medium) {
                if (currentEffectLevel == IceWaterLevel::Low)
                    isEmitting = al::isEffectEmitting(iceEffect, "EnterColdWaterLv1");
            } else {
                isEmitting = al::isEffectEmitting(iceEffect, "EnterColdWaterLv2");
            }
        } else {
            isEmitting = al::isEffectEmitting(iceEffect, "EnterColdWaterLv3");
        }

        if (isEmitting) {
            mIceWaterLevel = previousLevel;
        } else {
            iceEffect = mIceEffect;
            IceWaterLevel emitLevel = mIceWaterLevel;
            if (emitLevel != IceWaterLevel::High) {
                if (emitLevel != IceWaterLevel::Medium) {
                    if (emitLevel == IceWaterLevel::Low)
                        al::emitEffect(iceEffect, "EnterColdWaterLv1", nullptr);
                } else {
                    al::emitEffect(iceEffect, "EnterColdWaterLv2", nullptr);
                }
            } else {
                al::emitEffect(iceEffect, "EnterColdWaterLv3", nullptr);
            }
        }
    }

    al::LiveActor* player = mPlayer;
    s32 iceWaterCount = mIceWaterCount;
    s32 interval = mConst->getIceWaterDamageInterval();
    if (iceWaterCount != 0) {
        s32 remaining = interval - iceWaterCount % interval;
        if (remaining > IceWaterHighLevelFrames)
            al::holdSe(player, "ColdWaterLv");
        else
            al::holdSe(player, "ColdWaterHurryLv");
    }
}

bool PlayerCounterIceWater::isTriggerDamage() const {
    return mIceWaterCount != 0 && mRecoveryCount == 0 &&
           mIceWaterCount % mConst->getIceWaterDamageInterval() == 0;
}

void PlayerCounterIceWater::updateRecoveryCountImpl() {
    if (mIceWaterCount == 0)
        return;

    mRecoveryCount++;
    if (mRecoveryCount < mConst->getIceWaterRecoveryFrame() && !rs::isCollidedGround(mCollider))
        return;

    clearIceWaterCount();
}

void PlayerCounterIceWater::killIceEffect() {
    al::tryKillEmitterAndParticleAll(mIceEffect);
}
