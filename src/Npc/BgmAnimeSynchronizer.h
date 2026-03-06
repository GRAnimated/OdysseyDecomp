#pragma once

namespace al {
class ByamlIter;
class LiveActor;
}  // namespace al

class BgmAnimeSynchronizer {
public:
    static BgmAnimeSynchronizer* tryCreate(al::LiveActor*, const al::ByamlIter&);
    void trySyncBgm();
};
