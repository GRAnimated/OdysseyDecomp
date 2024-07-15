#pragma once

namespace al {
class HitSensor;
class SensorMsg;
}  // namespace al

namespace rs {

bool sendMsgEnemyAttack2D(al::HitSensor* source, al::HitSensor* target);
bool sendMsgNoticePlayerDamage(al::HitSensor* source, al::HitSensor* target);
bool sendMsgTouchFireDrum2D(al::HitSensor* source, al::HitSensor* target);
bool sendMsgItemAmiiboKoopa(al::HitSensor* source, al::HitSensor* target);
bool sendMsgPushToPlayer(al::HitSensor* source, al::HitSensor* target);

bool isMsgCapTouchWall(const al::SensorMsg*);
bool isMsgCapHipDrop(const al::SensorMsg*);
bool isMsgPlayerDisregardTargetMarker(const al::SensorMsg*);
bool isMsgPlayerDisregardHomingAttack(const al::SensorMsg*);
bool isMsgPlayerDisregard(const al::SensorMsg*);
bool isMsgCapAttackCollide(const al::SensorMsg*);
bool isMsgFrogHackTrample(const al::SensorMsg*);
bool isMsgCapAttack(const al::SensorMsg*);
bool isMsgPressDown(const al::SensorMsg*);
bool isMsgAttackDirect(const al::SensorMsg*);

}  // namespace rs
