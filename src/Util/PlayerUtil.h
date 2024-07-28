#pragma once

namespace al {
class LiveActor;
}

namespace rs {

bool isPlayerHack(const al::LiveActor*);
bool isPlayerHackGroupUseCameraStick(const al::LiveActor*);
bool isPlayerHackTRex(const al::LiveActor*);
}  // namespace rs
