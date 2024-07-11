#pragma once

namespace al {
class LiveActor;
}
class BarrierField;

namespace rs {
void setBossBarrierField(BarrierField*);
bool isActiveDemo(const al::LiveActor*);
}  // namespace rs
