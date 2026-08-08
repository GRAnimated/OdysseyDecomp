#include "Library/Obj/FootPrintServer.h"

#include <basis/seadNew.h>

#include "Library/LiveActor/ActorFlagFunction.h"
#include "Project/Obj/FootPrint.h"

namespace al {

FootPrintServer::FootPrintServer(const ActorInitInfo& initInfo, const char* name, s32 count) {
    _8 = new sead::PtrArray<FootPrint>();
    _8->allocBuffer(count, nullptr);
    for (s32 i = 0; i < _8->capacity(); i++)
        _8->pushBack(new FootPrint(initInfo, name));
}

FootPrint* FootPrintServer::findDeadFootPrint() {
    if (_8->size() < 1)
        return nullptr;

    for (s32 i = 0; i < _8->size(); i++) {
        FootPrint* footPrint = _8->at(i);
        if (isDead(reinterpret_cast<const LiveActor*>(footPrint)))
            return _8->at(i);
    }

    return nullptr;
}

const char* FootPrintServer::getSceneObjName() const {
    return "足あとサーバー";
}

FootPrintServer::~FootPrintServer() = default;

}  // namespace al
