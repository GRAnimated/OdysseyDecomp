#include "Movement/FlashingCtrl.h"

namespace al {

    bool FlashingCtrl::isNowFlashing() const {return mInterval <= mTime;};
    void FlashingCtrl::start(int interval) {
        field_11 = false;
        mStatus = false;
        mInterval = interval;
        mTime = 180;
    };
    int FlashingCtrl::getCurrentInterval() const {
        if (mStatus)
            return 8;
        if (mInterval < 90)
            return 6;
        return 10;
    };
    float FlashingCtrl::getFlashingAnimRate() const {
        float rate = 10.0f;

        if (mStatus)
            rate = 8.0f;
        else if (mInterval < 90)
            rate = 6.0f;

        return 2.0f / rate;
    };
};    // namespace al