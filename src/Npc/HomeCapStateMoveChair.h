#pragma once

#include "Library/Nerve/NerveStateBase.h"

namespace al {
class LiveActor;
class ParabolicPath;
}  // namespace al

namespace sead {
template <typename T>
struct Vector3;
using Vector3f = Vector3<f32>;
}  // namespace sead

class HomeCapStateMoveChair : public al::ActorStateBase {
public:
    HomeCapStateMoveChair(al::LiveActor* actor);

    void init() override;
    void appear() override;

    void appearMoveOtherChair(al::LiveActor* otherChair);
    void getCapAppearPos(sead::Vector3f* outPos);

    void exeMove();

private:
    al::ParabolicPath* mPath;
    al::LiveActor* mOtherChair;
};

static_assert(sizeof(HomeCapStateMoveChair) == 0x30);
