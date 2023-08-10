#pragma once

#include <prim/seadSafeString.h>
#include "al/Library/Audio/IUseAudioKeeper.h"
#include "al/Library/Camera/IUseCamera.h"
#include "al/Library/Effect/IUseEffectKeeper.h"
#include "al/Library/HostIO/HioNode.h"
#include "al/Library/Layout/IUseLayout.h"
#include "al/Library/Layout/IUseLayoutAction.h"
#include "al/Library/Message/IUseMessageSystem.h"
#include "al/Library/Nerve/IUseNerve.h"
#include "al/Library/Scene/SceneObjHolder.h"

namespace al {
class NerveKeeper;
class LayoutKeeper;
class LayoutActionKeeper;
class LayoutTextPaneAnimator;
class EffectKeeper;
class AudioKeeper;
class LayoutExecuteInfo;
class HitReactionKeeper;
class LayoutSceneInfo;
class LayoutPartsActorKeeper;
class CameraDirector;
class SceneObjHolder;
class MessageSystem;
class Nerve;

class LayoutActor : public IUseHioNode,
                    public IUseNerve,
                    public IUseLayout,
                    public IUseLayoutAction,
                    public IUseMessageSystem,
                    public IUseCamera,
                    public IUseAudioKeeper,
                    public IUseEffectKeeper,
                    public IUseSceneObjHolder {
public:
    LayoutActor(const char*);

    virtual void appear();
    virtual void kill();
    virtual void control() {}
    virtual void calcAnim(bool recursive);
    virtual void movement();

    virtual NerveKeeper* getNerveKeeper() const override;
    virtual const char* getName() const override;
    virtual EffectKeeper* getEffectKeeper() const override;
    virtual AudioKeeper* getAudioKeeper() const override;
    virtual LayoutActionKeeper* getLayoutActionKeeper() const override;
    virtual LayoutKeeper* getLayoutKeeper() const override;
    virtual CameraDirector* getCameraDirector() const override;
    virtual SceneObjHolder* getSceneObjHolder() const override;
    virtual const MessageSystem* getMessageSystem() const override;

    void initLayoutKeeper(LayoutKeeper*);
    void initActionKeeper();
    void initTextPaneAnimator(LayoutTextPaneAnimator*);
    void initExecuteInfo(LayoutExecuteInfo*);
    void initHitReactionKeeper(HitReactionKeeper*);
    void initSceneInfo(LayoutSceneInfo*);
    void initLayoutPartsActorKeeper(s32);
    void initEffectKeeper(EffectKeeper*);
    void initAudioKeeper(AudioKeeper*);
    void initNerve(const Nerve*, s32);
    void setMainGroupName(const char*);
    void syncAction();

    LayoutExecuteInfo* getExecuteInfo() { return mExecuteInfo; }
    
private:
    sead::FixedSafeString<0x80> mName;
    NerveKeeper* mNerveKeeper;
    LayoutKeeper* mLayoutKeeper;
    LayoutActionKeeper* mLayoutActionKeeper;
    LayoutTextPaneAnimator* mTextPaneAnimator;
    EffectKeeper* mEffectKeeper;
    AudioKeeper* mAudioKeeper;
    LayoutExecuteInfo* mExecuteInfo;
    HitReactionKeeper* mHitReactionKeeper;
    LayoutSceneInfo* mLayoutSceneInfo;
    LayoutPartsActorKeeper* mLayoutPartsActorKeeper;
    bool mIsAlive;
};
}  // namespace al