#include "Npc/PlayerRecoder.h"

#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Player/PlayerUtil.h"

#include "Util/PlayerUtil.h"

namespace {
NERVE_IMPL(PlayerRecoder, Wait);
NERVE_IMPL(PlayerRecoder, Recode);
NERVE_IMPL(PlayerRecoder, End);
NERVES_MAKE_NOSTRUCT(PlayerRecoder, Wait, Recode, End);
}  // namespace

PlayerRecoder::PlayerRecoder(const al::LiveActor* actor, s32 maxFrames)
    : al::NerveExecutor("プレイヤー移動を記録"), mActor(actor), mMaxFrames(maxFrames) {
    mEntries = new PlayerRecordEntry[maxFrames];
    initNerve(&Wait, 0);
}

PlayerRecoder::~PlayerRecoder() = default;

void PlayerRecoder::start() {
    mCurrentFrame = 0;
    al::setNerve(this, &Recode);
}

void PlayerRecoder::exeWait() {}

// NON_MATCHING: Vector3f/Quatf copy width differs (target: 8+4/8+8, ours: 4+4+4/4+4+4+4)
void PlayerRecoder::exeRecode() {
    al::LiveActor* playerActor = al::getPlayerActor(mActor, 0);

    const sead::Vector3f& playerPos = al::getPlayerPos(mActor, 0);
    mEntries[mCurrentFrame].trans = playerPos;

    const sead::Quatf& quat = al::getQuat(playerActor);
    mEntries[mCurrentFrame].posture = quat;

    sead::Vector3f capPos = sead::Vector3f::zero;
    mEntries[mCurrentFrame].hasCapPos = rs::tryGetFlyingCapPos(&capPos, mActor);
    mEntries[mCurrentFrame].capPos = capPos;

    mCurrentFrame++;
    if (al::isGreaterEqualStep(this, mMaxFrames))
        al::setNerve(this, &End);
}

void PlayerRecoder::exeEnd() {}

void PlayerRecoder::update() {
    updateNerve();
}

const sead::Vector3f& PlayerRecoder::getTrans(s32 index) const {
    return mEntries[index].trans;
}

const sead::Quatf& PlayerRecoder::getPosture(s32 index) const {
    return mEntries[index].posture;
}

const char* PlayerRecoder::getAnimName(s32 index) const {
    return mEntries[index].animName;
}

void PlayerRecoder::endRecord() {
    al::setNerve(this, &End);
}

bool PlayerRecoder::isEndRecode() const {
    return al::isNerve(this, &End);
}
