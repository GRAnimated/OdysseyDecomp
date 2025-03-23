#pragma once

#include "Library/LiveActor/LiveActor.h"

namespace al {
class BgmDataBase;

class BgmPlayObj : public al::LiveActor {
public:
    class PlayParams;

    BgmPlayObj(const char*, bool);
    void init(const al::ActorInitInfo&) override;
    void createShape(al::BgmDataBase*);
    void init(const al::ActorInitInfo&, const char*);
    void init(const al::ActorInitInfo&, const char*, const char*, const char*);
    void initAfterPlacement() override;
    void appear() override;
    void kill() override;
    void stopBgm(s32);
    void movement() override;
    void isEnableCalcSpeakerParam() const;
    void calc3DParams(bool);
    void finalize();
    void getDistanceFromSourceToListener();
    void isPlayable() const;
    void activate(bool, bool, bool);
    void startBgm(bool, bool);
    void activate(const al::BgmPlayObj::PlayParams&, bool);
    void deactivate(bool, s32);
    void exeWaitOnSwitch();
    void exeWaitPlayStart();
    void exePlay();

    void setMatrix(const sead::Matrix34f* matrix) {
        if (matrix)
            mMatrix = *matrix;
    }

private:
    void* filler[5];
    sead::Matrix34f mMatrix;
    void* filler2[17];
};
}  // namespace al
