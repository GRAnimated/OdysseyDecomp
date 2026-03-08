#pragma once

#include "Library/LiveActor/LiveActor.h"
#include "Library/Message/IUseMessageSystem.h"

class CheckpointFlag : public al::LiveActor, public al::IUseMessageSystem {
public:
    CheckpointFlag(const char* name);

    const al::MessageSystem* getMessageSystem() const override;

    bool isHomeFlag() const { return mIsHomeFlag; }

private:
    void* mMessageSystem = nullptr;
    void* mLinksPlayerActorInfo = nullptr;
    al::PlacementId* mPlacementId = nullptr;
    const char16_t* mNameLabel = nullptr;
    s32 _130 = 0;
    u8 _134 = 0;
    u8 mIsHomeFlag = 0;
    u8 mIsZeroGravity = 0;
    u8 _137 = 0;
    void* mAirBubble = nullptr;
    s32 _140 = -1;
    u32 _144 = 0;
    void* mBirdMtxGlideCtrl = nullptr;
};
