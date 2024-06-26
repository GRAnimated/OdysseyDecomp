#include "Library/Play/Layout/WipeSimple.h"

#include "Library/Layout/LayoutActionFunction.h"
#include "Library/Layout/LayoutActor.h"
#include "Library/Layout/LayoutInitInfo.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
using namespace al;
NERVE_IMPL(WipeSimple, Close);
NERVE_IMPL(WipeSimple, CloseEnd);
NERVE_IMPL(WipeSimple, Open);

NERVES_MAKE_NOSTRUCT(WipeSimple, Open);
NERVES_MAKE_STRUCT(WipeSimple, Close, CloseEnd);
}  // namespace

namespace al {

WipeSimple::WipeSimple(const char* name, const char* layoutName, const LayoutInitInfo& info,
                       const char* actorName)
    : LayoutActor(name) {
    initLayoutActor(this, info, layoutName, actorName);
    initNerve(&NrvWipeSimple.Close, 0);
}

void WipeSimple::startClose(s32 time) {
    mTime = time;
    startAction(this, "Appear", nullptr);
    LayoutActor::appear();

    al::setActionFrameRate(
        this, (mTime <= 0) ? 1.0f : al::getActionFrameMax(this, "Appear", nullptr) / mTime,
        nullptr);
    al::setNerve(this, &NrvWipeSimple.Close);
}

void WipeSimple::tryStartClose(s32 time) {
    if (isCloseEnd() ||
        (!al::isNerve(this, &NrvWipeSimple.Close) && !al::isNerve(this, &NrvWipeSimple.CloseEnd)))
        startClose(time);
}

void WipeSimple::startCloseEnd() {
    startAction(this, "Wait", nullptr);
    LayoutActor::appear();
    al::setNerve(this, &NrvWipeSimple.CloseEnd);
}

void WipeSimple::startOpen(s32 time) {
    mTime = time;
    al::startAction(this, "End", nullptr);
    al::setNerve(this, &Open);
}

void WipeSimple::tryStartOpen(s32 time) {
    if (isOpenEnd() || al::isNerve(this, &Open))
        return;

    startOpen(time);
}

bool WipeSimple::isCloseEnd() const {
    return al::isNerve(this, &NrvWipeSimple.CloseEnd);
}

bool WipeSimple::isOpenEnd() const {
    return !isAlive();
}

void WipeSimple::exeClose() {
    if (!al::isFirstStep(this) && al::isActionEnd(this, nullptr))
        al::setNerve(this, &NrvWipeSimple.CloseEnd);
}

void WipeSimple::exeCloseEnd() {
    if (al::isFirstStep(this))
        al::startAction(this, "Wait", nullptr);
}

void WipeSimple::exeOpen() {
    if (al::isFirstStep(this)) {
        al::setActionFrameRate(
            this, (mTime <= 0) ? 1.0f : al::getActionFrameMax(this, "End", nullptr) / mTime,
            nullptr);
    }
    if (al::isActionEnd(this, nullptr))
        kill();
}

void WipeSimple::appear() {
    LayoutActor::appear();
}

}  // namespace al
