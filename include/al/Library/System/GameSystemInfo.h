#pragma once

#include "basis/seadTypes.h"

class ProjectNfpDirector;
class ApplicationMessageReceiver;

namespace al {
    class EffectSystem;
    class LayoutSystem;
    class MessageSystem;
    class NetworkSystem;
    class AudioSystem;
    class GamePadSystem;
    class GameDrawInfo;
    class HtmlViewer;
    class WaveVibrationHolder;

    class GameSystemInfo {
    public:
        u64 _0;
        al::EffectSystem *mEffectSys;                     // 0x08
        al::LayoutSystem *mLayoutSys;                     // 0x10
        al::MessageSystem *mMessageSys;                   // 0x18
        al::NetworkSystem *mNetworkSys;                   // 0x20
        al::AudioSystem *mAudioSys;                       // 0x28
        al::GamePadSystem *mGamePadSys;                   // 0x30
        al::GameDrawInfo *mDrawInfo;                      // 0x38 from Application::sInstance + 0x30
        ProjectNfpDirector *mProjNfpDirector;             // 0x48
        al::HtmlViewer *mHtmlViewer;                      // 0x50
        ApplicationMessageReceiver *mMessageReciever;     // 0x58
        al::WaveVibrationHolder *mWaveVibrationHolder;    // 0x60
        u64 _68;
    };
}    // namespace al