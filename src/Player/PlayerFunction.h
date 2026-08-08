#pragma once

#include <container/seadPtrArray.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

#include "Player/PlayerCostumeInfo.h"
#include "Player/PlayerJointControlFollowMtxPtr.h"
#include "Player/PlayerJointControlPartsDynamics.h"

namespace al {
class LiveActor;
struct ActorInitInfo;
class ActorDitherAnimator;
class ActorParamHolder;
class AudioKeeper;
class Resource;
}  // namespace al

class PlayerConst;
class PlayerCostumeInfo;

namespace PlayerFunction {
PlayerCostumeInfo* initMarioModelActor(al::LiveActor* player, const al::ActorInitInfo& initInfo,
                                       const char* modelName, const char* capType,
                                       al::AudioKeeper* keeper, bool isCloset);
void setupMarioFaceEarringVisibility(al::LiveActor* actor, const PlayerCostumeInfo* costumeInfo);
void setupMarioHeadStrapVisibility(al::LiveActor* actor, const PlayerCostumeInfo* costumeInfo);
PlayerCostumeInfo* initMarioModelActorCloset(PlayerJointControlPartsDynamics**, al::LiveActor*,
                                             const al::ActorInitInfo&, const char*, const char*,
                                             const PlayerConst*, sead::Vector3f*, sead::Vector3f*,
                                             PlayerJointControlFollowMtxPtr**, sead::Matrix34f*);
PlayerCostumeInfo* initMarioModelActorDemo(PlayerJointControlPartsDynamics** jointCtrlPtr,
                                           al::LiveActor* player, const al::ActorInitInfo& initInfo,
                                           const char* bodyName, const char* capName,
                                           const PlayerConst* pConst, sead::Vector3f* noseScale,
                                           sead::Vector3f* earScale, bool isCloset);
void initMarioModelActor2D(al::LiveActor* actor, const al::ActorInitInfo& initInfo,
                           const char* model2DName, bool isInvisCap);
void initYoshiModelActor(al::LiveActor*, const al::ActorInitInfo&, const char*);
void createCapModelName(sead::BufferedSafeStringBase<char>* modelName, const char* playerModelName);
al::Resource* initCapModelActor(al::LiveActor*, const al::ActorInitInfo&, const char*);
al::Resource* initCapModelActorDemo(al::LiveActor*, const al::ActorInitInfo&, const char*);
void initYoshiTongueParamHolder(al::LiveActor* actor);
bool isNeedHairControl(const PlayerBodyCostumeInfo* bodyInfo, const char* headName);
bool isInvisibleCap(const PlayerCostumeInfo* costumeInfo);
void showHairVisibility(al::LiveActor* actor);
void hideHairVisibility(al::LiveActor* actor);
void syncBodyHairVisibility(al::LiveActor* actor, al::LiveActor* bodyActor);
void getMarioFaceNoseShrinkScale(sead::Vector3f* scale);
void getMarioFaceBigEarScale(sead::Vector3f* scale);
void syncMarioFaceBeardVisibility(al::LiveActor* actor, al::LiveActor* headActor);
void setupMarioFaceBeardVisibility(al::LiveActor* actor, const PlayerCostumeInfo* costumeInfo);
void syncMarioHeadStrapVisibility(al::LiveActor* actor);
void setupClosetPlayerModel(al::LiveActor*, al::LiveActor*, al::LiveActor**, sead::Vector3f*,
                            sead::Vector3f*, sead::Matrix34f*, sead::Vector3f*, const char**,
                            sead::Matrix34f*, sead::Vector3f*, PlayerJointControlFollowMtxPtr*,
                            const PlayerCostumeInfo*);
void updateClosetHeadPartsMtx(al::LiveActor*, const al::LiveActor*, const sead::Matrix34f*,
                              const sead::Vector3f&, const char*, const sead::Matrix34f*,
                              const sead::Vector3f&, bool, sead::Matrix34f*);
PlayerConst* createMarioConst(const char*);
PlayerConst* createYoshiConst(al::LiveActor*, const char*, const char*);
al::ActorDitherAnimator* createPlayerDitherAnimator(al::LiveActor* actor, f32 distance);
bool isPlayerHitPointOne(const al::LiveActor* actor);
bool isPlayerDeadStatus(const al::LiveActor* actor);
bool isPlayerDeadWipeStart(const al::LiveActor* actor);
bool isPlayerDeadEnableCoinAppear(const al::LiveActor* actor);
void getPlayerDeadWipeInfo(const al::LiveActor* actor, const char** name, s32* wait);
bool isPlayerDeadDrawForward(const al::LiveActor* actor);
u32 getPlayerInputPort(const al::LiveActor* actor);
const sead::Matrix34f* getPlayerViewMtx(const al::LiveActor* actor);
void calcPlayerInputVec(sead::Vector3f* inputVec, const al::LiveActor* actor);
bool tryActivateAmiiboPreventDamage(const al::LiveActor* actor);
const char* getPlayerDepthGroundShadowName();
void changeDepthShadowMapSizeHigh(al::LiveActor* actor);
void changeDepthShadowMapSizeNormal(al::LiveActor* actor);
void createPlayerStainDecorationPartsArray(sead::PtrArray<sead::SafeStringBase<char>>*,
                                           al::LiveActor*);
void validatePlayerStain(al::LiveActor*, const sead::PtrArray<sead::SafeStringBase<char>>&);
void invalidatePlayerStain(al::LiveActor*, const sead::PtrArray<sead::SafeStringBase<char>>&);
void setupPlayerStain(al::LiveActor*, const sead::PtrArray<sead::SafeStringBase<char>>&, s32,
                      const sead::Color4f&, f32, f32, f32, f32);
}  // namespace PlayerFunction
