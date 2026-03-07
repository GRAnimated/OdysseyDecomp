#pragma once

#include <basis/seadTypes.h>

namespace al {
class AudioDirector;
}

class SceneAudioSystemPauseController {
public:
    SceneAudioSystemPauseController(al::AudioDirector*);
    void update();
    void tryResume();
    void pause(bool);
    void resume(s32);

private:
    u8 _padding[0x10];
};

static_assert(sizeof(SceneAudioSystemPauseController) == 0x10);
