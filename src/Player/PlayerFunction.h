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
u32 getPlayerInputPort(const al::LiveActor*);
const sead::Matrix34f* getPlayerViewMtx(const al::LiveActor*);
bool tryActivateAmiiboPreventDamage(const al::LiveActor*);
bool isPlayerDeadStatus(const al::LiveActor* actor);
bool isPlayerHitPointOne(const al::LiveActor* actor);
const char* getPlayerDepthGroundShadowName();
void syncBodyHairVisibility(al::LiveActor*, al::LiveActor*);
void syncMarioFaceBeardVisibility(al::LiveActor*, al::LiveActor*);
void syncMarioHeadStrapVisibility(al::LiveActor*);
bool isNeedHairControl(const PlayerBodyCostumeInfo*, const char*);
bool isInvisibleCap(const PlayerCostumeInfo*);
void hideHairVisibility(al::LiveActor*);

void setupMarioFaceEarringVisibility(al::LiveActor*, const PlayerCostumeInfo*);
void setupMarioHeadStrapVisibility(al::LiveActor*, const PlayerCostumeInfo*);
PlayerCostumeInfo* initMarioModelActorCloset(PlayerJointControlPartsDynamics**, al::LiveActor*,
                                             const al::ActorInitInfo&, const char*, const char*,
                                             const PlayerConst*, sead::Vector3f*, sead::Vector3f*,
                                             PlayerJointControlFollowMtxPtr**, sead::Matrix34f*);
void initYoshiModelActor(al::LiveActor*, const al::ActorInitInfo&, const char*);
void initYoshiTongueParamHolder(al::LiveActor*);
void showHairVisibility(al::LiveActor*);
void getMarioFaceNoseShrinkScale(sead::Vector3f*);
void getMarioFaceBigEarScale(sead::Vector3f*);
void setupMarioFaceBeardVisibility(al::LiveActor*, const PlayerCostumeInfo*);
void setupClosetPlayerModel(al::LiveActor*, al::LiveActor*, al::LiveActor**, sead::Vector3f*,
                            sead::Vector3f*, sead::Matrix34f*, sead::Vector3f*, const char**,
                            sead::Matrix34f*, sead::Vector3f*, PlayerJointControlFollowMtxPtr*,
                            const PlayerCostumeInfo*);
void updateClosetHeadPartsMtx(al::LiveActor*, const al::LiveActor*, const sead::Matrix34f*,
                              const sead::Vector3f&, const char*, const sead::Matrix34f*,
                              const sead::Vector3f&, bool, sead::Matrix34f*);
PlayerConst* createYoshiConst(al::LiveActor*, const char*, const char*);
al::ActorDitherAnimator* createPlayerDitherAnimator(al::LiveActor*, f32);
bool isPlayerDeadWipeStart(const al::LiveActor*);
bool isPlayerDeadEnableCoinAppear(const al::LiveActor*);
void getPlayerDeadWipeInfo(const al::LiveActor*, const char**, s32*);
bool isPlayerDeadDrawForward(const al::LiveActor*);
void calcPlayerInputVec(sead::Vector3f*, const al::LiveActor*);
void changeDepthShadowMapSizeHigh(al::LiveActor*);
void changeDepthShadowMapSizeNormal(al::LiveActor*);
void createPlayerStainDecorationPartsArray(sead::PtrArray<sead::SafeStringBase<char>>*,
                                           al::LiveActor*);
void validatePlayerStain(al::LiveActor*, const sead::PtrArray<sead::SafeStringBase<char>>&);
void invalidatePlayerStain(al::LiveActor*, const sead::PtrArray<sead::SafeStringBase<char>>&);
void setupPlayerStain(al::LiveActor*, const sead::PtrArray<sead::SafeStringBase<char>>&, s32,
                      const sead::Color4f&, f32, f32, f32, f32);

PlayerConst* createMarioConst(const char*);
void createCapModelName(sead::BufferedSafeStringBase<char>*, const char*);

void initMarioModelActor2D(al::LiveActor* actor, const al::ActorInitInfo& initInfo,
                           const char* model2DName, bool isInvisCap);
al::Resource* initCapModelActor(al::LiveActor*, const al::ActorInitInfo&, const char*);
al::Resource* initCapModelActorDemo(al::LiveActor*, const al::ActorInitInfo&, const char*);
PlayerCostumeInfo* initMarioModelActor(al::LiveActor* player, const al::ActorInitInfo& initInfo,
                                       const char* modelName, const char* capType,
                                       al::AudioKeeper* keeper, bool isCloset);
PlayerCostumeInfo* initMarioModelActorDemo(PlayerJointControlPartsDynamics** jointCtrlPtr,
                                           al::LiveActor* player, const al::ActorInitInfo& initInfo,
                                           const char* bodyName, const char* capName,
                                           const PlayerConst* pConst, sead::Vector3f* noseScale,
                                           sead::Vector3f* earScale, bool isCloset);
}  // namespace PlayerFunction
