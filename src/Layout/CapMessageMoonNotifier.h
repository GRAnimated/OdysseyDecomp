#pragma once

namespace al {
class StageInfo;
struct ActorInitInfo;
}  // namespace al

class CapMessageMoonNotifier {
public:
    static void initialize(const al::StageInfo*, const al::ActorInitInfo&);
};
