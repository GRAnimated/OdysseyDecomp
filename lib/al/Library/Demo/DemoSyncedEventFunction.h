#pragma once

namespace al {
class AudioDirector;
class DemoSyncedEventKeeper;
}

namespace alDemoSyncedEventFunction {
void tryCreateAndSetSeKeeper(al::DemoSyncedEventKeeper* keeper, al::AudioDirector* director,
                             const char* name);
}
