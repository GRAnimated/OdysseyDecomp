#pragma once

#include "Library/LiveActor/LiveActor.h"

class DigPoint : public al::LiveActor {
public:
    DigPoint(const char* name);

    void init(const al::ActorInitInfo& info) override;
    void appear() override;
    void kill() override;

    void emitGlowEffect();
    bool tryUpdateHintTrans(const sead::Vector3f& trans);

    void* _108 = nullptr;
    bool _110 = false;
};
