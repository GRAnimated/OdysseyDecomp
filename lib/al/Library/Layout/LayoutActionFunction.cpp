#include "Library/Layout/LayoutActionFunction.h"
#include "Library/Base/StringUtil.h"
#include "Library/Layout/IUseLayoutAction.h"
#include "Library/Layout/LayoutActionKeeper.h"
#include "Library/Layout/LayoutActor.h"
#include "Library/Layout/LayoutPaneGroup.h"
#include "Library/Math/MathRandomUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace al {

void startAction(IUseLayoutAction* layout, const char* actionName, const char* paneName) {
    layout->getLayoutActionKeeper()->startAction(actionName, paneName);
}

void startActionAtRandomFrame(IUseLayoutAction* layout, const char* actionName,
                              const char* paneName) {
    startAction(layout, actionName, paneName);
    LayoutPaneGroup* paneGroup = layout->getLayoutActionKeeper()->getLayoutPaneGroup(paneName);
    f32 random = (s32)al::getRandom(0.0f, paneGroup->getAnimFrameMax());
    paneGroup->setAnimFrame(random);
}

void startFreezeAction(IUseLayoutAction* layout, const char* actionName, f32 frame,
                       const char* paneName) {
    startAction(layout, actionName, paneName);
    LayoutPaneGroup* paneGroup = layout->getLayoutActionKeeper()->getLayoutPaneGroup(paneName);
    paneGroup->setAnimFrame(frame);
    paneGroup->setAnimFrameRate(0.0f);
}

void startFreezeActionEnd(IUseLayoutAction* layout, const char* actionName, const char* paneName) {
    f32 animFrameMax = getActionFrameMax(layout, actionName, paneName);
    startAction(layout, actionName, paneName);
    LayoutPaneGroup* paneGroup = layout->getLayoutActionKeeper()->getLayoutPaneGroup(paneName);
    paneGroup->setAnimFrame(animFrameMax);
    paneGroup->setAnimFrameRate(0.0f);
}

f32 getActionFrameMax(const IUseLayoutAction* layout, const char* actionName,
                      const char* paneName) {
    return layout->getLayoutActionKeeper()->getLayoutPaneGroup(paneName)->getAnimFrameMax(
        actionName);
}

void startFreezeGaugeAction(IUseLayoutAction* layout, f32 value, f32 minFrame, f32 maxFrame,
                            const char* actionName, const char* paneName) {
    f32 frame =
        al::calcRate01(value, minFrame, maxFrame) * getActionFrameMax(layout, actionName, paneName);
    startAction(layout, actionName, paneName);
    LayoutPaneGroup* paneGroup = layout->getLayoutActionKeeper()->getLayoutPaneGroup(paneName);
    paneGroup->setAnimFrame(frame);
    paneGroup->setAnimFrameRate(0.0f);
}

bool tryStartAction(IUseLayoutAction* layout, const char* actionName, const char* paneName) {
    if (isExistAction(layout, actionName, paneName)) {
        startAction(layout, actionName, paneName);
        return true;
    }
    return false;
}

bool isExistAction(const IUseLayoutAction* layout, const char* actionName, const char* paneName) {
    return isExistAction(layout, paneName) &&
           layout->getLayoutActionKeeper()->getLayoutPaneGroup(paneName)->isAnimExist(actionName);
}

bool isActionEnd(const IUseLayoutAction* layout, const char* paneName) {
    LayoutPaneGroup* paneGroup = layout->getLayoutActionKeeper()->getLayoutPaneGroup(paneName);
    if (paneGroup && paneGroup->isAnimPlaying() && paneGroup->isAnimOneTime())
        return paneGroup->isAnimEnd();
    return true;
}

bool isExistAction(const IUseLayoutAction* layout, const char* actionName) {
    return layout->getLayoutActionKeeper()->getLayoutPaneGroup(actionName) != nullptr;
}

bool isActionOneTime(const IUseLayoutAction* layout, const char* actionName, const char* paneName) {
    return layout->getLayoutActionKeeper()->getLayoutPaneGroup(paneName)->isAnimOneTime(actionName);
}

f32 getActionFrame(const IUseLayoutAction* layout, const char* paneName) {
    return layout->getLayoutActionKeeper()->getLayoutPaneGroup(paneName)->getAnimFrame();
}

void setActionFrame(IUseLayoutAction* layout, f32 frame, const char* paneName) {
    layout->getLayoutActionKeeper()->getLayoutPaneGroup(paneName)->setAnimFrame(frame);
}

f32 getActionFrameRate(const IUseLayoutAction* layout, const char* paneName) {
    return layout->getLayoutActionKeeper()->getLayoutPaneGroup(paneName)->getAnimFrameRate();
}

void setActionFrameRate(IUseLayoutAction* layout, f32 frameRate, const char* paneName) {
    layout->getLayoutActionKeeper()->getLayoutPaneGroup(paneName)->setAnimFrameRate(frameRate);
}

const char* getActionName(const IUseLayoutAction* layout, const char* paneName) {
    return layout->getLayoutActionKeeper()->getLayoutPaneGroup(paneName)->getPlayingAnimName();
}

bool isActionPlaying(const IUseLayoutAction* layout, const char* actionName, const char* paneName) {
    return al::isEqualString(
        layout->getLayoutActionKeeper()->getLayoutPaneGroup(paneName)->getPlayingAnimName(),
        actionName);
}

bool isAnyActionPlaying(const IUseLayoutAction* layout, const char* paneName) {
    return layout->getLayoutActionKeeper()->getLayoutPaneGroup(paneName)->isAnimPlaying();
}

void setNerveAtActionEnd(LayoutActor* layout, const Nerve* nerve) {
    const IUseLayoutAction* iuseLayoutAction = layout;
    LayoutPaneGroup* paneGroup =
        iuseLayoutAction->getLayoutActionKeeper()->getLayoutPaneGroup(nullptr);

    if (!(paneGroup && paneGroup->isAnimPlaying() && paneGroup->isAnimOneTime() &&
          !paneGroup->isAnimEnd()))
        setNerve(layout, nerve);
}

}  // namespace al
