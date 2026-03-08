#pragma once

namespace al {
class LiveActor;
}

namespace CapManHeroDemoUtil {
void initCapManHeroTailJointController(al::LiveActor* actor);
void invalidateDitherAnimIfExist(al::LiveActor* actor);
}  // namespace CapManHeroDemoUtil
