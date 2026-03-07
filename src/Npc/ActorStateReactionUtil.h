#pragma once

namespace al {
class SensorMsg;
}  // namespace al

class ActorStateReactionBase;

namespace ActorStateReactionUtil {
bool isInvalidRestartCapReaction(const al::SensorMsg* msg,
                                 const ActorStateReactionBase* reaction);
}  // namespace ActorStateReactionUtil
