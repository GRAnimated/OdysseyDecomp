#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Area/AreaObj.h"
#include "Library/Area/IUseAreaObj.h"
#include "Library/HostIO/HioNode.h"

namespace al {

class AudioDirector;
class ModelKeeper;
class CameraDirector;
class AreaObjDirector;
class AudioEventController;
class AudioEffectController;
class AudioRequestKeeperSyncedBgm;
class AudioMic;
class PlayerHolder;
class SeKeeper;
class BgmKeeper;

class AudioKeeper : public HioNode {
public:
    AudioKeeper(const AudioDirector* audioDirector);
    void initSeKeeper(const AudioDirector* audioDirector, const char* name,
                      const sead::Vector3f* position, const sead::Matrix34f* emitterMtx,
                      const ModelKeeper* modelKeeper, CameraDirector* cameraDirector);
    void initBgmKeeper(const AudioDirector* audioDirector, const char* name);

    void validate();
    void invalidate();
    void startClipped();
    void endClipped();
    void appear();
    void kill();
    virtual ~AudioKeeper();

    AudioEventController* getAudioEventController() const { return mAudioEventController; }

    AudioEffectController* getAudioEffectController() const { return mAudioEffectController; }

    AudioRequestKeeperSyncedBgm* getAudioRequestKeeperSyncedBgm() const {
        return mAudioRequestKeeperSyncedBgm;
    }

    SeKeeper* getSeKeeper() const { return mSeKeeper; }

    BgmKeeper* getBgmKeeper() const { return mBgmKeeper; }

    AudioMic* getAudioMic() const { return mAudioMic; }

private:
    AudioEventController* mAudioEventController = nullptr;
    AudioEffectController* mAudioEffectController = nullptr;
    AudioRequestKeeperSyncedBgm* mAudioRequestKeeperSyncedBgm = nullptr;
    SeKeeper* mSeKeeper = nullptr;
    BgmKeeper* mBgmKeeper = nullptr;
    AudioMic* mAudioMic = nullptr;
};

static_assert(sizeof(AudioKeeper) == 0x38);

class AudioGeneralPurposeAreaChecker : public HioNode, public IUseAreaObj {
public:
    AudioGeneralPurposeAreaChecker(const char* areaName);

    void init(AreaObjDirector* areaObjDirector);

    void reset();
    void update();

    bool tryFindAreaObjPlayerOne(AreaObj** area) const;

    bool isInArea() const;
    bool isInvaridByOneTime() const;

    void setPlayerHolder(const PlayerHolder* playerHolder);

    s32 getIntArgInCurArea(const char* key) const;
    f32 getFloatArgInCurArea(const char* key) const;
    bool getBoolArgInCurArea(const char* key) const;

    const char* getStringArgInCurArea(const char* key) const;
    const char* getStringArgInCurAreaWithAreaCheck(const char* key) const;

    s32 getIntArgInPastArea(const char* key) const;
    const char* getStringArgInPastArea(const char* key) const;

    bool tryGetIntArgInCurArea(s32* out, const char* key) const;
    bool tryGetStringArgInCurArea(const char** out, const char* key) const;

    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

    bool isEnteredArea() const { return mIsEnteredArea; }

    bool isExitedArea() const { return mIsExitedArea; }

private:
    const char* mAreaName;

    const AreaObj* mCurArea = nullptr;
    const AreaObj* mPrevArea = nullptr;

    bool _20 = false;
    u8 _21[7];

    AreaObjDirector* mAreaObjDirector = nullptr;
    const PlayerHolder* mPlayerHolder = nullptr;

    bool mIsEnteredArea = false;
    bool mIsExitedArea = false;
    bool mIsAreaChanged = false;
    u8 mIsCurAreaActive = 0;
};

static_assert(sizeof(AudioGeneralPurposeAreaChecker) == 0x40);

}  // namespace al

namespace alAudioKeeperFunction {
al::AudioKeeper* createAudioKeeper(const al::AudioDirector* audioDirector);
al::AudioKeeper* createAudioKeeper(const al::AudioDirector* audioDirector, const char* seName,
                                   const char* bgmName);
}  // namespace alAudioKeeperFunction
