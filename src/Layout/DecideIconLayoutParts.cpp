#include "Layout/DecideIconLayoutParts.h"

#include "Library/Layout/LayoutActionFunction.h"
#include "Library/Layout/LayoutActor.h"
#include "Library/Layout/LayoutActorUtil.h"
#include "Library/Layout/LayoutInitInfo.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Util/StageInputFunction.h"

namespace {
NERVE_IMPL(DecideIconLayoutParts, End);
NERVE_IMPL(DecideIconLayoutParts, Appear);
NERVE_IMPL(DecideIconLayoutParts, Wait);
NERVE_IMPL(DecideIconLayoutParts, Decide);
NERVE_IMPL(DecideIconLayoutParts, DecideAfter);

NERVES_MAKE_NOSTRUCT(DecideIconLayoutParts, End, Appear, Wait, Decide, DecideAfter);
}  // namespace

DecideIconLayoutParts::DecideIconLayoutParts(const char* name, al::LayoutActor* parentActor,
                                             const al::LayoutInitInfo& initInfo)
    : al::LayoutActor(name) {
    if (parentActor)
        al::initLayoutPartsActor(this, parentActor, initInfo, "ParHardKey", nullptr);
    initNerve(&End, 0);
}

void DecideIconLayoutParts::start() {
    al::setNerve(this, &Appear);
}

void DecideIconLayoutParts::exeAppear() {
    if (al::isFirstStep(this))
        al::startAction(this, "Appear");
    if (al::isActionEnd(this))
        al::setNerve(this, &Wait);
}

void DecideIconLayoutParts::exeWait() {
    if (al::isFirstStep(this))
        al::startAction(this, "Wait");
    if (rs::isTriggerUiDecide(this) || rs::isTriggerUiCancel(this))
        al::setNerve(this, &Decide);
}

void DecideIconLayoutParts::exeDecide() {
    if (al::isFirstStep(this))
        al::startAction(this, "PageNext");
    if (al::isActionEnd(this))
        al::setNerve(this, &DecideAfter);
}

void DecideIconLayoutParts::exeDecideAfter() {
    if (al::isGreaterEqualStep(this, 10))
        al::setNerve(this, &End);
}

void DecideIconLayoutParts::exeEnd() {}

bool DecideIconLayoutParts::isDecide() const {
    return al::isNerve(this, &Decide);
}

bool DecideIconLayoutParts::isWait() const {
    return al::isNerve(this, &Wait);
}

bool DecideIconLayoutParts::isEnd() const {
    return al::isNerve(this, &End);
}
