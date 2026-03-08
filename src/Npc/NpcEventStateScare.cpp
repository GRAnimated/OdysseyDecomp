#include "Npc/NpcEventStateScare.h"

#include "Library/Event/EventFlowExecutor.h"
#include "Library/Event/EventFlowUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Util/NpcAnimUtil.h"

namespace {
NERVE_IMPL(NpcEventStateScare, Scare);
NERVE_IMPL(NpcEventStateScare, ScareStart);
NERVE_IMPL(NpcEventStateScare, ScareEnd);
NERVES_MAKE_NOSTRUCT(NpcEventStateScare, Scare, ScareStart, ScareEnd);
}  // namespace

NpcEventStateScareActionParam::NpcEventStateScareActionParam(const char* scareAction)
    : _0(scareAction) {}

NpcEventStateScareActionParam::NpcEventStateScareActionParam(const char* scareStartAction,
                                                             const char* scareAction,
                                                             const char* scareEndAction)
    : _0(scareAction), _8(scareStartAction), _10(scareEndAction) {}

NpcEventStateScare::NpcEventStateScare(al::LiveActor* actor,
                                       const NpcEventStateScareActionParam* param)
    : al::ActorStateBase("イベント中の怖がり", actor), _28(param) {
    initNerve(&Scare, 0);
}

void NpcEventStateScare::setActionParam(const NpcEventStateScareActionParam* param) {
    _28 = param;
}

bool NpcEventStateScare::tryStart(const al::EventFlowExecutor* executor) {
    if (!al::isScare(executor))
        return false;
    _20 = executor;
    if (_28 && _28->_8)
        al::setNerve(this, &ScareStart);
    else
        al::setNerve(this, &Scare);
    return true;
}

void NpcEventStateScare::kill() {
    al::NerveStateBase::kill();
    _20 = nullptr;
}

void NpcEventStateScare::exeScareStart() {
    if (al::isFirstStep(this))
        rs::startNpcAction(mActor, _28->_8);
    if (al::isActionEnd(mActor)) {
        if (al::isScare(_20))
            al::setNerve(this, &Scare);
        else
            al::setNerve(this, &ScareEnd);
    }
}

void NpcEventStateScare::exeScare() {
    if (al::isFirstStep(this)) {
        if (_28 && _28->_0)
            rs::startNpcAction(mActor, _28->_0);
    }
    if (!al::isScare(_20)) {
        if (_28 && _28->_10)
            al::setNerve(this, &ScareEnd);
        else
            kill();
    }
}

void NpcEventStateScare::exeScareEnd() {
    if (al::isFirstStep(this))
        rs::startNpcAction(mActor, _28->_10);
    if (al::isActionEnd(mActor))
        kill();
}
