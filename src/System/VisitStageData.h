#pragma once

#include <basis/seadTypes.h>

#include "System/ByamlSave.h"

class VisitStageData : public ByamlSave {
public:
    VisitStageData();
    void init();
    void checkAlreadyVisit(const char* stageName) const;
    void visit(const char* stageName);
    void write(al::ByamlWriter* writer) override;
    void read(const al::ByamlIter& iter) override;

private:
    void* field_8 = nullptr;
    s32 dword_10 = 0;
};
