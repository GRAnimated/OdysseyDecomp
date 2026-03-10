#pragma once

namespace al {
class HitSensor;
class LiveActor;
}  // namespace al

namespace YukimaruMovement {
bool attackSensor(al::LiveActor* actor, al::HitSensor* self, al::HitSensor* other);
void updateVelocity(al::LiveActor* actor);
}  // namespace YukimaruMovement
