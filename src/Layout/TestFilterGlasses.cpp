#include "Layout/TestFilterGlasses.h"
#include "Library/Layout/LayoutActionFunction.h"
#include "Library/Layout/LayoutInitInfo.h"

/*

class TestFilterGlasses : public al::LayoutActor {
public:
    TestFilterGlasses(const char*, const al::LayoutInitInfo&, const char*);

    void startAppear();
    void end();

    void exeAppear();
    void exeWait();
    void exeEnd();
    bool isEnd() const;
};


*/

#include "Library/Nerve/NerveSetupUtil.h"

namespace {
NERVE_IMPL(TestFilterGlasses, Appear);
NERVE_IMPL(TestFilterGlasses, End);
NERVE_IMPL(TestFilterGlasses, Wait);

NERVES_MAKE_NOSTRUCT(TestFilterGlasses, Appear, End, Wait);
}  // namespace

TestFilterGlasses::TestFilterGlasses(const char* name, const al::LayoutInitInfo& info,
                                     const char* suffix)
    : al::LayoutActor(name) {
    al::initLayoutActor(this, info, "FilterGlasses", suffix);
    initNerve(&Appear, 0);
    kill();
}

void TestFilterGlasses::startAppear() {
    appear();
    al::setNerve(this, &Appear);
}

void TestFilterGlasses::end() {
    al::setNerve(this, &End);
}

void TestFilterGlasses::exeAppear() {
    if (al::isFirstStep(this))
        al::startAction(this, "Appear", nullptr);
    if (al::isActionEnd(this, nullptr))
        al::setNerve(this, &Wait);
}

void TestFilterGlasses::exeWait() {
    if (al::isFirstStep(this))
        al::startAction(this, "Wait", nullptr);
}

void TestFilterGlasses::exeEnd() {
    if (al::isFirstStep(this))
        al::startAction(this, "End", nullptr);
    if (al::isActionEnd(this, nullptr))
        kill();
}

bool TestFilterGlasses::isEnd() const {
    return !isAlive() || al::isNerve(this, &End);
}
