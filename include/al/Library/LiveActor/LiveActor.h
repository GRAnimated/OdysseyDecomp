#pragma once

#include "al/Library/HitSensor/HitSensorKeeper.h"
#include "al/Library/IUse/IUse.h"
#include "al/Project/HitSensor/HitSensor.h"
#include <math/seadMatrix.h>

namespace al {
    class ActorActionKeeper;
    class ActorExecuteInfo;
    class ActorItemKeeper;
    class ActorPrePassLightKeeper;
    class ActorPoseKeeperBase;
    class ActorOcclusionKeeper;
    class ActorSceneInfo;
    class ActorScoreKeeper;
    class Collider;
    class ModelKeeper;
    class RailKeeper;
    class SensorMsg;
    class ScreenPointer;
    class ScreenPointTarget;
    class ShadowKeeper;
    class ScreenPointKeeper;
    class SubActorKeeper;

    class LiveActorFlag {
    public:
        LiveActorFlag();

        bool mIsDead = true;    // _0
        bool _1 = false;
        bool _2 = true;
        bool _3 = false;
        bool mIsOffCalcAnim = false;    // _4
        bool mIsHideModel = false;      // _5
        bool mIsNoCollide = true;       // _6
        bool _7 = false;
        bool mIsMaterialCodeValid = false;            // _8
        bool mIsPuddleMaterialValid = false;          // _9
        bool mIsAreaTarget = true;                    // _A
        bool mIsUpdateMvmntEffAudioColSnsr = true;    // _B
    };

    class LiveActor : public IUseNerve,
                      public IUseEffectKeeper,
                      public IUseAudioKeeper,
                      public IUseStageSwitch,
                      public IUseSceneObjHolder,
                      public IUseAreaObj,
                      public IUseCamera,
                      public IUseCollision,
                      public IUseRail,
                      public IUseHioNode {

        virtual NerveKeeper *getNerveKeeper() const;

        virtual void initAfterPlacement();
        virtual void appear();
        virtual void makeActorAlive();
        virtual void kill();
        virtual void makeActorDead();
        virtual void movement();
        virtual void calcAnim();
        virtual void draw();
        virtual void startClipped();
        virtual void endClipped();
        virtual void attackSensor(HitSensor *, HitSensor *);
        virtual bool receiveMsg(const SensorMsg *, HitSensor *, HitSensor *);
        virtual bool receiveMsgScreenPoint(const SensorMsg *, ScreenPointer *, ScreenPointTarget *);
        virtual const char *getName() const;
        virtual sead::Matrix34f &getBaseMtx();
        virtual EffectKeeper *getEffectKeeper() const;
        virtual AudioKeeper *getAudioKeeper() const;
        virtual StageSwitchKeeper *getStageSwitchKeeper() const;
        virtual RailRider *getRailRider() const;
        virtual SceneObjHolder *getSceneObjHolder() const;
        virtual CollisionDirector *getCollisionDirector() const;
        virtual AreaObjDirector *getAreaObjDirector() const;
        virtual CameraDirector *getCameraDirector() const;
        virtual void initStageSwitchKeeper();
        virtual void control();
        virtual void updateCollider();

        const char *mActorName;                   // _48
        ActorPoseKeeperBase *mPoseKeeper;         // _50
        ActorExecuteInfo *mExecuteInfo;           // _58
        ActorActionKeeper *mActorActionKeeper;    // _60
        ActorItemKeeper *mItemKeeper;             // _68
        ActorScoreKeeper *mScoreKeeper;           // _70
        Collider *mCollider;                      // _78
        u64 *_80;
        ModelKeeper *mModelKeeper;                // _88
        NerveKeeper *mNerveKeeper;                // _90
        HitSensorKeeper *mSensorKeeper;           // _98
        ScreenPointKeeper *mScreenPointKeeper;    // _A0
        EffectKeeper *mEffectKeeper;              // _A8
        AudioKeeper *mAudioKeeper;                // _B0
        u64 *_B8;
        StageSwitchKeeper *mSwitchKeeper;          // _C0
        RailKeeper *mRailKeeper;                   // _C8
        ShadowKeeper *mShadowKeeper;               // _D0
        ActorPrePassLightKeeper *mLightKeeper;     // _D8
        ActorOcclusionKeeper *mOcclusionKeeper;    // _E0
        SubActorKeeper *mSubActorKeeper;           // _E8
        u64 *_F0;
        ActorSceneInfo *mSceneInfo;    // _F8
        LiveActorFlag *mFlags;         // _100
    };
};    // namespace al