#include "Library/Sequence/Sequence.h"

namespace al {

void Sequence::init(const SequenceInitInfo& info) {}

void Sequence::kill() {
    mIsAlive = false;
}

}  // namespace al
