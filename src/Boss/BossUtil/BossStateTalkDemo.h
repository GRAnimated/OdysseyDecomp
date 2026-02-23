#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

#include "Library/Nerve/NerveStateBase.h"

#include "Demo/IUseDemoSkip.h"

namespace al {
struct ActorInitInfo;
class CameraTicket;
class EventFlowExecutor;
class IEventFlowEventReceiver;
class LiveActor;
}  // namespace al

class BossStateTalkDemo : public al::ActorStateBase, public IUseDemoSkip {
public:
    static BossStateTalkDemo* createWithEventFlow(al::LiveActor* owner,
                                                  const al::ActorInitInfo& initInfo,
                                                  const char* eventFlowName,
                                                  const char* cameraSuffix);
    static BossStateTalkDemo* createWithEventFlow(al::LiveActor* owner,
                                                  const al::ActorInitInfo& initInfo,
                                                  const char* eventFlowName);

    BossStateTalkDemo(const char* name, al::LiveActor* owner, const al::ActorInitInfo& initInfo,
                      al::EventFlowExecutor* executor);

    bool tryStart(const char* entryName);
    void startWithoutRequestDemo(const char* entryName);
    void setEventReceiver(al::IEventFlowEventReceiver* receiver);

    void kill() override;
    bool update() override;

    bool isFirstDemo() const override;
    bool isEnableSkipDemo() const override;
    void skipDemo() override;

    void exeDemo();
    void exeDemoTalkFirst();
    void exeDemoTalk();
    void exeEnd();
    void exeSkip();

    bool isPrevTalkDemo() const;
    bool isStartActionTiming() const;
    const char* getStartActionName() const;

    void setEnableSkipDemo(bool enable) { mIsEnableSkipDemo = enable; }

private:
    al::EventFlowExecutor* mEventFlowExecutor = nullptr;
    const char* mDemoEntryName = nullptr;
    al::CameraTicket* mDemoAnimCamera = nullptr;
    sead::BufferedSafeStringBase<char>* mDemoActionName = nullptr;
    bool mIsRequestedDemo;
    bool mIsEnableSkipDemo;
    bool mIsFirstDemo;
    bool mIsReplaced;
    sead::Vector3f mReplaceTrans;
    sead::Quatf mReplaceQuat;
};

static_assert(sizeof(BossStateTalkDemo) == 0x68);
