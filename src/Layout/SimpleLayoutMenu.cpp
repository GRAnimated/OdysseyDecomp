#include "Layout/SimpleLayoutMenu.h"
#include "Library/Layout/LayoutActionFunction.h"
#include "Library/Layout/LayoutInitInfo.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
NERVE_HOST_TYPE_IMPL(SimpleLayoutMenu, Appear);
NERVE_HOST_TYPE_IMPL(SimpleLayoutMenu, Wait);
NERVE_HOST_TYPE_IMPL(SimpleLayoutMenu, End);
NERVE_HOST_TYPE_IMPL(SimpleLayoutMenu, EndWait);

NERVES_MAKE_STRUCT(HostType, Appear, End, EndWait, Wait);
}  // namespace

SimpleLayoutMenu::SimpleLayoutMenu(const char* name, const char* name2,
                                   const al::LayoutInitInfo& info, const char* name3, bool localize)
    : al::LayoutActor(name) {
    if (localize)
        al::initLayoutActorLocalized(this, info, name2, name3);
    else
        al::initLayoutActor(this, info, name2, name3);

    initNerve(&NrvHostType.Appear, 0);
}

SimpleLayoutMenu::SimpleLayoutMenu(al::LayoutActor* parent, const char* name, const char* name2,
                                   const al::LayoutInitInfo& info, const char* name3)
    : al::LayoutActor(name) {
    al::initLayoutPartsActor(this, parent, info, name2, name3);
    initNerve(&NrvHostType.Appear, 0);
}

void SimpleLayoutMenu::startAppear(const char* actionName) {
    al::startAction(this, actionName, nullptr);
    al::LayoutActor::appear();
    setNerve(this, &NrvHostType.Appear);
}

void SimpleLayoutMenu::startEnd(const char* actionName) {
    if (al::isNerve(this, &NrvHostType.End) || isEndWait())
        return;
    al::startAction(this, actionName, nullptr);
    al::setNerve(this, &NrvHostType.End);
}

void SimpleLayoutMenu::exeAppear() {
    if (al::isActionEnd(this, nullptr))
        al::setNerve(this, &NrvHostType.Wait);
}

void SimpleLayoutMenu::exeWait() {
    if (al::isFirstStep(this))
        al::startAction(this, "Wait", nullptr);
    if ((field_12C & 0x80000000) == 0 && al::isGreaterEqualStep(this, field_12C))
        al::setNerve(this, &NrvHostType.End);
}

void SimpleLayoutMenu::exeEnd() {
    if (al::isActionEnd(this, nullptr))
        al::setNerve(this, &NrvHostType.EndWait);
}

void SimpleLayoutMenu::exeEndWait() {}

bool SimpleLayoutMenu::isAppearOrWait() const {
    return isWait() || al::isNerve(this, &NrvHostType.Appear);
}

bool SimpleLayoutMenu::isWait() const {
    return al::isNerve(this, &NrvHostType.Wait);
}

bool SimpleLayoutMenu::isEndWait() const {
    return al::isNerve(this, &NrvHostType.EndWait);
}