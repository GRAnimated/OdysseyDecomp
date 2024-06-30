#pragma once

namespace al {
class AudioSystemInfo;
class Sequence;

class AudioDirectorInitInfo {
public:
    al::AudioSystemInfo* mAudioSystemInfo;
    al::Sequence* mCurSequence;
    char* mCurStage;
    int mScenarioNo;
    void* qword20;
    char mSeDirectorInitInfo[104];
    char mBgmDirectorInitInfo[16];
    char* mDuckingName;
};

}  // namespace al
