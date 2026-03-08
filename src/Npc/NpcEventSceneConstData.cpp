#include "Npc/NpcEventSceneConstData.h"

// NON_MATCHING: sead FixedSafeString<32> constructor chain — compiler optimizes away
// intermediate vtable assignments and dead code in clear() that the target retains
NpcEventSceneConstData::NpcEventSceneConstData() = default;
