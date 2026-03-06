#pragma once

namespace al {
class ByamlIter;

class EventFlowActorParamHolder {
public:
    EventFlowActorParamHolder();

    void load(const ByamlIter& iter);
    void* findSuffixParam(const char* suffix) const;

    void* mParam = nullptr;
    void** mParamArray = nullptr;
    s32 mParamCount = 0;
};
}  // namespace al
