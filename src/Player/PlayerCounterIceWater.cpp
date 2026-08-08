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

void PlayerCounterIceWater::clearIceWaterCount() {
    al::LiveActor* iceEffect = mIceEffect;
    const s32 level = mIceWaterLevel;
    if (level != 3) {
        if (level != 2) {
            if (level == 1)
                al::tryDeleteEffect(iceEffect, "EnterColdWaterLv1");
        } else {
            al::tryDeleteEffect(iceEffect, "EnterColdWaterLv2");
        }
    } else {
        al::tryDeleteEffect(iceEffect, "EnterColdWaterLv3");
    }
    mIceWaterCount = 0;
    mRecoveryCount = 0;
    mIceWaterLevel = 0;
    mIsInIceWater = false;
}

void PlayerCounterIceWater::updateCount(bool isInIceWater, bool isOnGround) {
    const bool isJudge = rs::updateJudgeAndResult(mJudge);
    const s32 previousLevel = mIceWaterLevel;
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

        const s32 remaining = mConst->getIceWaterDamageInterval() - mIceWaterCount;
        if (remaining <= 90)
            mIceWaterLevel = 3;
        else if (remaining <= 180)
            mIceWaterLevel = 2;
        else
            mIceWaterLevel = 1;
        mRecoveryCount = 0;
    } else {
        if (isOnGround && isTriggerDamage())
            mIceWaterCount++;
        updateRecoveryCountImpl();
    }

    if (previousLevel != mIceWaterLevel) {
        al::LiveActor* iceEffect = mIceEffect;
        const s32 previousEffectLevel = previousLevel < 1 ? 1 : previousLevel;
        if (previousEffectLevel != 3) {
            if (previousEffectLevel != 2) {
                if (previousEffectLevel == 1)
                    al::tryDeleteEffect(iceEffect, "EnterColdWaterLv1");
            } else {
                al::tryDeleteEffect(iceEffect, "EnterColdWaterLv2");
            }
        } else {
            al::tryDeleteEffect(iceEffect, "EnterColdWaterLv3");
        }

        iceEffect = mIceEffect;
        const s32 currentEffectLevel = mIceWaterLevel;
        bool isEmitting = false;
        if (currentEffectLevel != 3) {
            if (currentEffectLevel != 2) {
                if (currentEffectLevel == 1)
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
            const s32 emitLevel = mIceWaterLevel;
            if (emitLevel != 3) {
                if (emitLevel != 2) {
                    if (emitLevel == 1)
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
    const s32 iceWaterCount = mIceWaterCount;
    const s32 interval = mConst->getIceWaterDamageInterval();
    if (iceWaterCount != 0) {
        const s32 remaining = interval - iceWaterCount % interval;
        if (remaining >= 91)
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

    al::LiveActor* iceEffect = mIceEffect;
    const s32 level = mIceWaterLevel;
    if (level != 3) {
        if (level != 2) {
            if (level == 1)
                al::tryDeleteEffect(iceEffect, "EnterColdWaterLv1");
        } else {
            al::tryDeleteEffect(iceEffect, "EnterColdWaterLv2");
        }
    } else {
        al::tryDeleteEffect(iceEffect, "EnterColdWaterLv3");
    }
    mIceWaterCount = 0;
    mRecoveryCount = 0;
    mIceWaterLevel = 0;
    mIsInIceWater = false;
}

void PlayerCounterIceWater::killIceEffect() {
    al::tryKillEmitterAndParticleAll(mIceEffect);
}
