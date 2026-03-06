#include "Npc/TalkNpcParamHolder.h"

#include "Library/LiveActor/ActorModelFunction.h"

#include "Npc/TalkNpcParam.h"

TalkNpcParamHolder::TalkNpcParamHolder() = default;

TalkNpcParam* TalkNpcParamHolder::findOrCreateParam(const al::LiveActor* actor,
                                                    const char* suffix) {
    if (al::isExistModel(actor) && mParamCount >= 1) {
        for (s32 i = 0; i < mParamCount; i++) {
            if (mParams[i]->isEqualModelName(actor) && mParams[i]->isEqualSuffixName(suffix)) {
                TalkNpcParam* param = mParams[i];
                if (param)
                    return param;
                break;
            }
        }
    }

    if (mParamCount < 1) {
        mParams = new TalkNpcParam*[64];
        for (s32 i = 0; i < 64; i++)
            mParams[i] = nullptr;
    }

    TalkNpcParam* param = new TalkNpcParam();
    param->init(actor, suffix);
    mParams[mParamCount] = param;
    mParamCount++;
    return param;
}

TalkNpcParam* TalkNpcParamHolder::tryFindParamLocal(const al::LiveActor* actor,
                                                    const char* suffix) const {
    if (!al::isExistModel(actor) || mParamCount < 1)
        return nullptr;

    for (s32 i = 0; i < mParamCount; i++) {
        if (mParams[i]->isEqualModelName(actor) && mParams[i]->isEqualSuffixName(suffix))
            return mParams[i];
    }
    return nullptr;
}

const char* TalkNpcParamHolder::getSceneObjName() const {
    return "トークNPCパラメータ保持";
}
