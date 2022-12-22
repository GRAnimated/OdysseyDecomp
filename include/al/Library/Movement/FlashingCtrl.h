#pragma once

#include "LiveActor/LiveActor.h"
#include "LiveActor/ActorModelFunction.h"
#include "Se/SeKeeper.h"

namespace al {
    class FlashingCtrl {
    public:
        FlashingCtrl(al::LiveActor *, bool, bool);
        virtual void movement();
        void end();
        bool isNowFlashing() const;
        void updateFlashing();
        void start(int);
        int getCurrentInterval() const;
        float getFlashingAnimRate() const;
        bool isNowJustFlashed() const;
        bool isNowOn() const;

        al::LiveActor *mActor;
        bool field_10;
        bool field_11;
        bool mStatus;
        int mInterval;
        int mTime;
        bool field_1C;
        bool field_1D;
    };
}    // namespace al