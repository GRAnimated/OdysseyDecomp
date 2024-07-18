#pragma once

#include "Library/Bgm/BgmDirectorInitInfo.h"
#include "Library/Se/SeDirectorInitInfo.h"

namespace al {
class AudioSystemInfo;
class Sequence;

class AudioDirectorInitInfo {
public:
    AudioDirectorInitInfo() {}

    AudioSystemInfo* mAudioSystemInfo = nullptr;
    Sequence* mCurSequence = nullptr;
    const char* mCurStage = nullptr;
    int mScenarioNo = 0;
    void* qword20 = nullptr;
    SeDirectorInitInfo mSeDirectorInitInfo;
    BgmDirectorInitInfo mBgmDirectorInitInfo;
    const char* mDuckingName = nullptr;
};

}  // namespace al
