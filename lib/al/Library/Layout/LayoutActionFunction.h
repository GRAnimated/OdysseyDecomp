#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

namespace al {
class IUseAudioKeeper;
class IUseLayoutAction;
class LayoutActor;
class MessageTagDataHolder;
class Nerve;
class ReplaceTagProcessorBase;

void startAction(IUseLayoutAction* layout, const char* actionName, const char* paneName);
void startActionAtRandomFrame(IUseLayoutAction* layout, const char* actionName,
                              const char* paneName);
void startFreezeAction(IUseLayoutAction* layout, const char* actionName, f32 frame,
                       const char* paneName);
void startFreezeActionEnd(IUseLayoutAction* layout, const char* actionName, const char* paneName);

f32 getActionFrameMax(const IUseLayoutAction* layout, const char* actionName, const char* paneName);

void startFreezeGaugeAction(IUseLayoutAction* layout, f32 value, f32 minFrame, f32 maxFrame,
                            const char* actionName, const char* paneName);

bool tryStartAction(IUseLayoutAction* layout, const char* actionName, const char* paneName);

f32 getActionFrame(const IUseLayoutAction* layout, const char* paneName);
void setActionFrame(IUseLayoutAction* layout, f32 frame, const char* paneName);
f32 getActionFrameRate(const IUseLayoutAction* layout, const char* paneName);
void setActionFrameRate(IUseLayoutAction* layout, f32 frameRate, const char* paneName);

const char* getActionName(const al::IUseLayoutAction* layout, const char* paneName);

bool isActionOneTime(const IUseLayoutAction* layout, const char* actionName, const char* paneName);
bool isActionPlaying(const IUseLayoutAction* layout, const char* actionName, const char* paneName);
bool isAnyActionPlaying(const IUseLayoutAction* layout, const char* paneName);
bool isActionEnd(const IUseLayoutAction* layout, const char* paneName);

bool isExistAction(const IUseLayoutAction* layout, const char* actionName);
bool isExistAction(const IUseLayoutAction* layout, const char* actionName, const char* paneName);

void setNerveAtActionEnd(LayoutActor*, const Nerve* nerve);

void startTextPaneAnim(al::LayoutActor*, const char16*, const al::MessageTagDataHolder*,
                       const al::ReplaceTagProcessorBase*);
void startTextPaneAnimWithAudioUser(al::LayoutActor*, const char16*,
                                    const al::MessageTagDataHolder*,
                                    const al::ReplaceTagProcessorBase*, al::IUseAudioKeeper const*);
void startAndSetTextPaneAnimStage(al::LayoutActor*, const char*, const char*,
                                  const al::MessageTagDataHolder*,
                                  const al::ReplaceTagProcessorBase*);
void startAndSetTextPaneAnimSystem(al::LayoutActor*, const char*, const char*,
                                   const al::MessageTagDataHolder*,
                                   const al::ReplaceTagProcessorBase*);
void endTextPaneAnim(al::LayoutActor*);
void skipTextPaneAnim(al::LayoutActor*);
void flushTextPaneAnim(al::LayoutActor*);
void changeNextPage(al::LayoutActor*, const al::MessageTagDataHolder*,
                    const al::ReplaceTagProcessorBase*);
void tryChangeNextPage(al::LayoutActor*, const al::MessageTagDataHolder*,
                       const al::ReplaceTagProcessorBase*);
bool isExistNextPage(const al::LayoutActor*);
bool isEndTextPaneAnim(const al::LayoutActor*, bool);
void getCurrentMessagePaneAnim(const al::LayoutActor*);
void calcCurrentMessageTextNum(const al::LayoutActor*);
void calcShowTextTime(s32);
void tryStartTextAnim(al::LayoutActor*, const char16*);
void tryStartTextTagVoice(al::LayoutActor*, const char16*, const al::IUseAudioKeeper*, const char*,
                          sead::FixedSafeString<64>*);
void startHitReaction(const al::LayoutActor*, const char*, const char*);
}  // namespace al
