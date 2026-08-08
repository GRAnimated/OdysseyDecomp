#pragma once

#include <container/seadPtrArray.h>

#include "Library/Scene/ISceneObj.h"

namespace al {
struct ActorInitInfo;
class FootPrint;
class FootPrintHolder;

class FootPrintServer : public ISceneObj {
public:
    FootPrintServer(const ActorInitInfo& initInfo, const char* name, s32 count);
    FootPrint* findDeadFootPrint();
    const char* getSceneObjName() const override;
    ~FootPrintServer() override;

private:
    friend class FootPrintHolder;
    sead::PtrArray<FootPrint>* _8;
};
static_assert(sizeof(FootPrintServer) == 0x10);
}  // namespace al
