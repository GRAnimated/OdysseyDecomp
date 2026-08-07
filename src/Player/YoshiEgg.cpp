#include "Player/YoshiEgg.h"

#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorClippingFunction.h"
#include "Library/LiveActor/ActorInitUtil.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorSensorUtil.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Placement/PlacementFunction.h"

#include "MapObj/FukankunZoomTargetFunction.h"
#include "Util/SensorMsgFunction.h"

class FukankunZoomCapMessage {
public:
    explicit FukankunZoomCapMessage(const al::LiveActor* actor);

    void init(const al::ActorInitInfo& info, const char* suffix, const char* targetName);
    void initAfterPlacement();
    void update();

    void setWatchCount(s32 watchCount) {
        mWatchCount = watchCount;
        mWatchStep = 0;
        mMode = 0;
    }

private:
    const al::LiveActor* mActor;
    s32 mWatchStep;
    s32 mMode;
    s32 mWatchCount;
    bool mIsActive;
    u8 _15[3];
    sead::Vector3f mOffset;
    u32 _24;
    void* _28;
    bool _30;
    u8 _31[7];
    void* _38;
    void* _40;
    bool _48;
    u8 _49[7];
};

static_assert(sizeof(FukankunZoomCapMessage) == 0x50);

YoshiEgg::YoshiEgg(const al::LiveActor* host, const IUsePlayerCollision* collision)
    : LiveActor("YoshiEgg"), mHost(host), mCollision(collision) {}

namespace {
NERVE_IMPL(YoshiEgg, Wait);
NERVE_IMPL(YoshiEgg, Appear);
NERVE_IMPL(YoshiEgg, Break);
NERVES_MAKE_STRUCT(YoshiEgg, Wait, Break);
YoshiEggNrvAppear Appear;
}  // namespace


void YoshiEgg::init(const al::ActorInitInfo& info) {
    al::initChildActorWithArchiveNameNoPlacementInfo(this, info, "YoshiEgg", nullptr);
    al::initNerve(this, &NrvYoshiEgg.Wait, 0);

    if (al::isPlaced(info)) {
        bool isFukankunZoomCapMessage = false;
        if (al::tryGetArg(&isFukankunZoomCapMessage, info, "IsFukankunZoomCapMessage") &&
            isFukankunZoomCapMessage) {
            mZoomCapMessage = new FukankunZoomCapMessage(this);
            mZoomCapMessage->init(info, "CapMessage", "FukankunZoomYoshiEgg");
            mZoomCapMessage->setWatchCount(
                FukankunZoomTargetFunction::getFukankunWatchCountDefault());
        }
    }

    makeActorDead();
}

void YoshiEgg::initAfterPlacement() {
    if (mZoomCapMessage)
        mZoomCapMessage->initAfterPlacement();
}

void YoshiEgg::initPlacementEgg() {
    al::copyPose(this, mHost);
    appear();
    al::setNerve(this, &NrvYoshiEgg.Wait);
}

void YoshiEgg::appearEgg() {
    al::copyPose(this, mHost);
    appear();
    al::invalidateClipping(this);
    al::setNerve(this, &Appear);
}

bool YoshiEgg::isEndAppear() const {
    return !al::isNerve(this, &Appear);
}

bool YoshiEgg::isBreak() const {
    return al::isNerve(this, &NrvYoshiEgg.Break);
}

void YoshiEgg::exeAppear() {
    if (al::isFirstStep(this))
        al::startAction(this, "Appear");
    al::copyPose(this, mHost);
    if (al::isActionEnd(this)) {
        al::validateClipping(this);
        al::setNerve(this, &NrvYoshiEgg.Wait);
    }
}

void YoshiEgg::exeWait() {
    al::tryStartActionIfNotPlaying(this, "Wait");
    al::copyPose(this, mHost);
    if (mZoomCapMessage)
        mZoomCapMessage->update();
}

void YoshiEgg::exeBreak() {
    if (al::isFirstStep(this))
        al::startAction(this, "Break");
    if (al::isActionEnd(this))
        kill();
}

void YoshiEgg::attackSensor(al::HitSensor* self, al::HitSensor* other) {
    if (al::isSensorName(self, "Push") && !al::isNerve(this, &NrvYoshiEgg.Break))
        rs::sendMsgPushToPlayer(other, self);
}

bool YoshiEgg::receiveMsg(const al::SensorMsg* message, al::HitSensor* other,
                          al::HitSensor* self) {
    if (al::isSensorName(self, "Push"))
        return al::isMsgPlayerDisregard(message);

    if (!(al::isMsgPlayerTrample(message) || rs::isMsgPlayerAndCapObjHipDropReflectAll(message) ||
          rs::isMsgCapAttack(message) || rs::isMsgThrowObjHitReflect(message) ||
          rs::isMsgTankBullet(message) || rs::isMsgMotorcycleAttack(message)) ||
        !al::isNerve(this, &NrvYoshiEgg.Wait))
        return false;

    al::setNerve(this, &NrvYoshiEgg.Break);
    return true;
}
