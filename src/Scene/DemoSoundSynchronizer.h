#pragma once

namespace al {
class AudioDirector;
class ApplicationMessageReceiver;
}

class DemoSoundSynchronizer {
public:
    DemoSoundSynchronizer(al::ApplicationMessageReceiver* receiver, al::AudioDirector* director);
};
