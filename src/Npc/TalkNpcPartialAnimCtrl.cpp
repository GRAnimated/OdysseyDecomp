#include "Npc/TalkNpcPartialAnimCtrl.h"

#include <cmath>
#include <math/seadMathCalcCommon.h>
#include <math/seadQuat.h>

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Npc/NpcStateReactionParam.h"
#include "Util/PlayerUtil.h"

namespace {
NERVE_IMPL(TalkNpcPartialAnimCtrl, None);
NERVE_END_IMPL(TalkNpcPartialAnimCtrl, ReactionCap);
NERVE_END_IMPL(TalkNpcPartialAnimCtrl, Reaction);
NERVE_IMPL(TalkNpcPartialAnimCtrl, ByeBye);
NERVES_MAKE_NOSTRUCT(TalkNpcPartialAnimCtrl, None, ReactionCap, Reaction, ByeBye);
}  // namespace

TalkNpcPartialAnimCtrl::TalkNpcPartialAnimCtrl(al::LiveActor* actor,
                                               const TalkNpcActionAnimInfo* animInfo)
    : al::NerveExecutor("トークNPCの部分アニメ制御"), mActor(actor), mAnimInfo(animInfo) {
    initNerve(&None, 0);
}

bool TalkNpcPartialAnimCtrl::isReactionCap() const {
    return al::isNerve(this, &ReactionCap);
}

void TalkNpcPartialAnimCtrl::update() {
    updateNerve();
}

void TalkNpcPartialAnimCtrl::forceEndAndInvalidateByeBye() {
    _21 = true;
    if (al::isNerve(this, &None))
        return;
    if (al::isPartialSklAnimAttached(mActor, 0)) {
        _44 = 120;
        al::clearPartialSklAnim(mActor, 0);
    }
    al::setNerve(this, &None);
}

void TalkNpcPartialAnimCtrl::forceEndAndInvalidateReaction() {
    _20 = false;
    if (al::isNerve(this, &None))
        return;
    if (al::isPartialSklAnimAttached(mActor, 0)) {
        _44 = 120;
        al::clearPartialSklAnim(mActor, 0);
    }
    al::setNerve(this, &None);
}

void TalkNpcPartialAnimCtrl::startReaction() {
    if (!_20)
        return;
    al::LiveActor* actor = mActor;
    NpcStateReactionParam* param = mReactionParam;
    al::startPartialSklAnim(actor, param->mReactionAnim.cstr(), 0, 0, nullptr);
    al::startHitReaction(mActor, "リアクション[踏み]");
    al::setNerve(this, &Reaction);
}

void TalkNpcPartialAnimCtrl::startReactionCap() {
    if (!_20)
        return;
    al::LiveActor* actor = mActor;
    NpcStateReactionParam* param = mReactionParam;
    al::startPartialSklAnim(actor, param->mReactionEndAnim.cstr(), 0, 0, nullptr);
    al::startHitReaction(mActor, "リアクション[帽子]");
    al::setNerve(this, &ReactionCap);
}

// NON_MATCHING: fmul operand order in quat rotation inline, b.gt/b.hi condition codes
void TalkNpcPartialAnimCtrl::exeNone() {
    if (!_21) {
        const char* jointName = mByeByeBaseJointName;
        if (jointName) {
            al::LiveActor* actor = mActor;
            const sead::Vector3f& playerPos = rs::getPlayerPos(actor);
            sead::Vector3f diff = playerPos - al::getTrans(actor);
            al::verticalizeVec(&diff, al::getGravity(actor), diff);

            if (diff.length() <= 1500.0f) {
                sead::Vector3f dir = diff;
                if (al::tryNormalizeOrZero(&dir)) {
                    sead::Quatf quat = sead::Quatf::unit;
                    al::calcJointQuat(&quat, actor, jointName);
                    sead::Vector3f frontDir = mByeByeLocalAxisFront;
                    frontDir.rotate(quat);

                    f32 dot = dir.dot(frontDir);
                    if (dot >= sead::Mathf::cos(sead::Mathf::deg2rad(15.0f))) {
                        sead::Vector3f projected = diff;
                        al::verticalizeVec(&projected, frontDir, projected);
                        if (projected.length() < 800.0f) {
                            if (_44 < 0) {
                                if (al::getRandom() < 0.33f) {
                                    al::setNerve(this, &ByeBye);
                                    return;
                                }
                            }
                            _44 = 120;
                            return;
                        }
                    }
                }
            }
        }
    }

    if (_44 >= 0)
        _44--;
}

void TalkNpcPartialAnimCtrl::exeReaction() {
    if (!al::isPartialSklAnimAttached(mActor, 0)) {
        al::setNerve(this, &None);
        return;
    }
    if (al::isPartialSklAnimEnd(mActor, 0))
        al::setNerve(this, &None);
}

void TalkNpcPartialAnimCtrl::endReaction() {
    al::clearPartialSklAnimWithInterpolate(mActor, 0, 15);
    _44 = 120;
}

void TalkNpcPartialAnimCtrl::exeReactionCap() {
    if (!al::isPartialSklAnimAttached(mActor, 0)) {
        al::setNerve(this, &None);
        return;
    }
    if (al::isPartialSklAnimEnd(mActor, 0))
        al::setNerve(this, &None);
}

void TalkNpcPartialAnimCtrl::endReactionCap() {
    al::clearPartialSklAnimWithInterpolate(mActor, 0, 15);
    _44 = 120;
}

void TalkNpcPartialAnimCtrl::exeByeBye() {
    if (al::isFirstStep(this)) {
        al::startPartialSklAnimWithInterpolate(mActor, "ByeBye", 0, 0, 8, nullptr);
        al::startHitReaction(mActor, "手を振る");
    }
    if (al::isPartialSklAnimEnd(mActor, 0)) {
        _44 = 120;
        al::clearPartialSklAnimWithInterpolate(mActor, 0, 15);
        al::setNerve(this, &None);
    }
}
