#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
}

class PlayerEquipmentUser;

namespace PlayerEquipmentFunction {
bool isEquipmentNoCapThrow(const PlayerEquipmentUser* equipmentUser);
void tryNoticeEquipPlayerDamage(PlayerEquipmentUser* equipmentUser);
bool isEquipmentForceDash(const PlayerEquipmentUser* equipmentUser);
bool tryGetEquipmentForceDashInfo(s32* frames, f32* speed,
                                  const PlayerEquipmentUser* equipmentUser);
}

namespace rs {
bool isPlayerSafetyPointRecovery(const al::LiveActor* actor);
}
