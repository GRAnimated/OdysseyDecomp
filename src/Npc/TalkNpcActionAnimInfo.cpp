#include "Npc/TalkNpcActionAnimInfo.h"

#include <new>
#include <prim/seadSafeString.h>

#include "Library/Base/StringUtil.h"
#include "Library/LiveActor/ActorActionFunction.h"
#include "Library/LiveActor/ActorAnimFunction.h"
#include "Library/LiveActor/ActorModelFunction.h"
#include "Library/LiveActor/ActorResourceFunction.h"
#include "Library/Math/MathUtil.h"
#include "Library/Placement/PlacementFunction.h"
#include "Library/Yaml/ByamlIter.h"
#include "Library/Yaml/ByamlUtil.h"

#include "Npc/NpcStateReactionParam.h"
#include "Npc/TalkNpcParam.h"
#include "Util/NpcAnimUtil.h"

struct NpcActionAnimParam {
    const char* waitAction;
    const char* walkAction;
    const char* talkAction;
    const char* turnAction;
    const char* turnL90Action;
    const char* turnR90Action;
    const char* turn180Action;
    const char* reactionAction;
    const char* reactionCapAction;
    const char* giveAction;
    const char* excitedAction;
    const char* scaredStartAction;
    const char* scaredAction;
    const char* scaredEndAction;
    const char* visAnimName;
};

// clang-format off
static const NpcActionAnimParam sNpcActionAnimParamTable[] = {
    {"AngryWait", "Walk", "AngryTalk", "Turn", "TurnL90", "TurnR90", "Turn180", "Reaction", "ReactionCap", "Give", "Excited", "ScaredStart", "Scared", "ScaredEnd", nullptr},
    {"CookKnife", "Walk", "Talk", "Turn", "TurnL90", "TurnR90", "Turn180", "Reaction", "ReactionCap", "Give", "Excited", "ScaredStart", "Scared", "ScaredEnd", "Knife"},
    {"CookLadle", "Walk", "TalkLadle", "Turn", "TurnL90Ladle", "TurnR90Ladle", "Turn180Ladle", "Reaction", "ReactionCap", "Give", "Excited", "ScaredStart", "Scared", "ScaredEnd", "Ladle"},
    {"CookLadlePan", "Walk", "Talk", "Turn", "TurnL90", "TurnR90", "Turn180", "ReactionLadlePan", "ReactionCapLadlePan", "Give", "ExcitedLadlePan", "ScaredStart", "ScaredLadlePan", "ScaredEnd", "LadlePan"},
    {"CookPan", "Walk", "Talk", "Turn", "TurnL90", "TurnR90", "Turn180", "Reaction", "ReactionCap", "Give", "ExcitedPan", "ScaredStart", "ScaredPan", "ScaredEnd", "Pan"},
    {"MusicWait", "Walk", "Talk", "Turn", "MusicTurnL90", "MusicTurnR90", "MusicTurn180", "Reaction", "ReactionCap", "Give", "Excited", "ScaredStart", "Scared", "ScaredEnd", nullptr},
    {"ResultLoseNpc", "Walk", "Talk", "Turn", "TurnL90", "TurnR90", "Turn180", "SitReaction", "SitReactionCap", "Give", "Excited", "ScaredStart", "SitScared", "ScaredEnd", nullptr},
    {"SadWait", "Walk", "SadTalk", "Turn", "TurnL90", "TurnR90", "Turn180", "Reaction", "ReactionCap", "Give", "Excited", "ScaredStart", "Scared", "ScaredEnd", nullptr},
    {"SitBenchExcited", "Walk", "Talk", "Turn", "TurnL90", "TurnR90", "Turn180", "SitBenchReaction", "SitBenchReactionCap", "SitBenchGive", "Excited", "ScaredStart", "SitScared", "ScaredEnd", nullptr},
    {"SitBenchTalk", "Walk", "Talk", "Turn", "TurnL90", "TurnR90", "Turn180", "SitBenchReaction", "SitBenchReactionCap", "SitBenchGive", "Excited", "ScaredStart", "SitScared", "ScaredEnd", nullptr},
    {"SitBenchWait", "Walk", "Talk", "Turn", "TurnL90", "TurnR90", "Turn180", "SitBenchReaction", "SitBenchReactionCap", "SitBenchGive", "Excited", "ScaredStart", "SitScared", "ScaredEnd", nullptr},
    {"SitScared", "Walk", "Talk", "Turn", "TurnL90", "TurnR90", "Turn180", "SitReaction", "SitReactionCap", "Give", "Excited", "ScaredStart", "SitScared", "ScaredEnd", nullptr},
    {"SitTalk", "Walk", "Talk", "Turn", "TurnL90", "TurnR90", "Turn180", "SitReaction", "SitReactionCap", "Give", "Excited", "ScaredStart", "SitScared", "ScaredEnd", nullptr},
    {"SitWait", "Walk", "Talk", "Turn", "TurnL90", "TurnR90", "Turn180", "SitReaction", "SitReactionCap", "Give", "Excited", "ScaredStart", "SitScared", "ScaredEnd", nullptr},
    {"SitWaitA", "Walk", "Talk", "Turn", "TurnL90", "TurnR90", "Turn180", "SitReaction", "SitReactionCap", "Give", "Excited", "ScaredStart", "SitScared", "ScaredEnd", nullptr},
    {"SitWaitB", "Walk", "Talk", "Turn", "TurnL90", "TurnR90", "Turn180", "SitReaction", "SitReactionCap", "Give", "Excited", "ScaredStart", "SitScared", "ScaredEnd", nullptr},
    {"SwimWait", "SwimWalk", "SwimTalk", "SwimTurn", "SwimTurnL90", "SwimTurnR90", "SwimTurn180", "SwimReaction", "SwimReactionCap", "Give", "Excited", "ScaredStart", "SwimScared", "ScaredEnd", nullptr},
    {"SwimWaitHappy", "SwimWalk", "SwimTalk", "SwimTurn", "SwimTurnL90", "SwimTurnR90", "SwimTurn180", "SwimReaction", "SwimReactionCap", "Give", "Excited", "ScaredStart", "SwimScared", "ScaredEnd", nullptr},
    {"SwimWaitHappyTalk", "SwimWalk", "SwimTalk", "SwimTurn", "SwimTurnL90", "SwimTurnR90", "SwimTurn180", "SwimReaction", "SwimReactionCap", "Give", "Excited", "ScaredStart", "SwimScared", "ScaredEnd", nullptr},
    {"SwimWaitSad", "SwimWalk", "SwimTalk", "SwimTurn", "SwimTurnL90", "SwimTurnR90", "SwimTurn180", "SwimReaction", "SwimReactionCap", "Give", "Excited", "ScaredStart", "SwimScared", "ScaredEnd", nullptr},
    {"SwimWalkWait", "SwimWalk", "SwimTalk", "SwimWalkTurn", "TurnL90", "TurnR90", "Turn180", "SwimWalkReaction", "SwimWalkReactionCap", "Give", "Excited", "ScaredStart", "SwimScared", "ScaredEnd", nullptr},
    {"UmbrellaWait", "Walk", "Talk", "Turn", "TurnL90", "TurnR90", "Turn180", "UmbrellaReaction", "UmbrellaReactionCap", "Give", "Excited", "ScaredStart", "Scared", "ScaredEnd", nullptr},
    {"WaitLadle", "Walk", "TalkLadle", "Turn", "TurnL90Ladle", "TurnR90Ladle", "Turn180Ladle", "Reaction", "ReactionCap", "Give", "Excited", "ScaredStart", "Scared", "ScaredEnd", "Ladle"},
};

static NpcActionAnimParam sDefaultNpcActionAnimParam =
    {"Wait", "Walk", "Talk", "Turn", "TurnL90", "TurnR90", "Turn180", "Reaction", "ReactionCap", "Give", "Excited", "ScaredStart", "Scared", "ScaredEnd", nullptr};

static NpcActionAnimParam sRabbitNpcActionAnimParam =
    {"WaitNpc", "RunNpc", "Talk", "TurnNpc", "TurnL90", "TurnR90", "Turn180", "ReactionNpc", "ReactionCapNpc", "Give", "Excited", "ScaredStart", "Scared", "ScaredEnd", nullptr};

static NpcActionAnimParam sRankingNpcActionAnimParam =
    {"Wait", "Walk", "Talk", "Wait", "TurnL90", "TurnR90", "Turn180", "Reaction", "ReactionCap", "Give", "Excited", "ScaredStart", "Scared", "ScaredEnd", nullptr};

TalkNpcActionAnimInfo::TalkNpcActionAnimInfo() = default;

const char* TalkNpcActionAnimInfo::getArgWaitActionName(const al::ActorInitInfo& initInfo) {
    return al::getStringArg(initInfo, "EventWaitActionName");
}

void TalkNpcActionAnimInfo::initWaitActionNameFromPlacementInfo(
    const al::LiveActor* actor, const al::ActorInitInfo& initInfo, bool isCheckHackWait) {
    al::tryGetStringArg(&_28, initInfo, "HackingWaitActionName");
    const char* waitAction = al::getStringArg(initInfo, "EventWaitActionName");
    initWaitActionNameDirect(actor, waitAction, isCheckHackWait);
}

// NON_MATCHING: loop codegen differences (32-bit counter, branch structure, relative addressing)
void TalkNpcActionAnimInfo::initWaitActionNameDirect(const al::LiveActor* actor,
                                                     const char* actionName,
                                                     bool isCheckHackWait) {
    mWaitActionName = actionName;

    if (!isCheckHackWait)
        return;

    if (al::isEqualString(al::getModelName(actor), "ForestManScrap"))
        return;

    if (al::isEqualString(al::getModelName(actor), "ForestMan")) {
        mWaitActionName = "ExcitedWithRandomA";
        return;
    }

    if (al::isEqualString(al::getModelName(actor), "CityMan") ||
        al::isEqualString(al::getModelName(actor), "CityWoman")) {
        _38 = "ExcitedMaxJump";
        return;
    }

    const char* waitAction = mWaitActionName;
    const NpcActionAnimParam* entry = sNpcActionAnimParamTable;
    const NpcActionAnimParam* found;
#pragma clang loop unroll(disable)
    for (s32 i = 0; i < 23; i++) {
        bool match = al::isEqualString(entry->waitAction, waitAction);
        found = entry;
        if (match)
            break;
        entry++;
        found = &sDefaultNpcActionAnimParam;
    }
    _38 = found->excitedAction;
}

// NON_MATCHING: loop codegen, relative addressing of special params, code size
void TalkNpcActionAnimInfo::init(const al::LiveActor* actor, const al::ActorInitInfo& initInfo,
                                 const TalkNpcParam* param, const char* suffix) {
    _8 = actor;

    al::ByamlIter iter;
    if (al::tryGetActorInitFileIter(&iter, actor, "NpcActionAnimCtrl", suffix) &&
        (s32)iter.getSize() >= 1) {
        for (s32 i = 0; ; ) {
            al::ByamlIter entryIter;
            if (iter.tryGetIterByIndex(&entryIter, i)) {
                if (al::isEqualString(mWaitActionName,
                                      al::getByamlKeyString(entryIter, "Name"))) {
                    al::tryGetByamlString(&mWaitActionName, entryIter, "WaitActionName");

                    al::ByamlIter randomIter;
                    if (al::tryGetByamlIterByKey(&randomIter, entryIter, "RandomActionName") &&
                        (s32)randomIter.getSize() >= 1) {
                        s32 count = randomIter.getSize();
                        _50 = count;
                        _58 = new const char*[count];

                        if (count >= 1) {
                            randomIter.tryGetStringByIndex(&_58[0], 0);
                            if (_50 >= 2) {
                                for (s32 j = 1; j < _50; j++)
                                    randomIter.tryGetStringByIndex(&_58[j], j);
                            }
                        }
                    }
                    break;
                }
            }
            if (++i >= (s32)iter.getSize())
                break;
        }
    }

    const char* effectiveWait = getWaitActionName();
    _60 = param->isInvalidChangeTurnAnimFromWait(effectiveWait);
    _61 = param->isInvalidChangeAllAnimFromWait(effectiveWait);

    if (al::isEqualString(al::getModelName(actor), "Rabbit")) {
        _10 = &sRabbitNpcActionAnimParam;
        mWaitActionName = _10->waitAction;
    } else if (al::isEqualString(al::getModelName(actor), "RankingNpc")) {
        _10 = &sRankingNpcActionAnimParam;
        mWaitActionName = _10->waitAction;
    } else {
        const char* waitAction = mWaitActionName;
        const NpcActionAnimParam* entry = sNpcActionAnimParamTable;
        const NpcActionAnimParam* found;
#pragma clang loop unroll(disable)
        for (s32 i = 0; i < 23; i++) {
            bool match = al::isEqualString(entry->waitAction, waitAction);
            found = entry;
            if (match)
                break;
            entry++;
            found = &sDefaultNpcActionAnimParam;
        }
        _10 = found;
    }

    auto* reactionParam = new NpcStateReactionParam();
    _18 = reactionParam;
    rs::makeNpcActionName(&_18->mReactionAnim, actor, _10->reactionAction);
    rs::makeNpcActionName(&_18->mReactionEndAnim, actor, _10->reactionCapAction);
}

const char* TalkNpcActionAnimInfo::getWaitActionName() const {
    if (_30) {
        if (_40)
            return _40;
        if (_28)
            return _28;
    }
    if (_38)
        return _38;
    if (_48)
        return _48;
    return mWaitActionName;
}

const char* TalkNpcActionAnimInfo::tryGetActorParamSuffix() const {
    if (al::isEqualString(_10->waitAction, "SwimWalkWait"))
        return "SwimWalk";
    return nullptr;
}

bool TalkNpcActionAnimInfo::tryApplyVisAnim(al::LiveActor* actor) const {
    const char* visAnimName = _10->visAnimName;
    if (!visAnimName)
        return false;
    al::startVisAnim(actor, visAnimName);
    return true;
}

const char* TalkNpcActionAnimInfo::convertActionName(sead::BufferedSafeStringBase<char>* buffer,
                                                     const char* actionName) const {
    const char* result = actionName;

    if (al::isEqualString(actionName, "Wait")) {
        result = getWaitActionName();
    } else if (al::isEqualString(actionName, "ScaredStart")) {
        result = _10->scaredStartAction;
    } else if (al::isEqualString(actionName, "Scared")) {
        result = _10->scaredAction;
    } else if (al::isEqualString(actionName, "ScaredEnd")) {
        result = _10->scaredEndAction;
    } else if (_61 && !rs::isOneTimeNpcAction(_8, actionName)) {
        result = getWaitActionName();
    } else if (al::isEqualString(actionName, "Turn")) {
        if (_60 || _61)
            result = getWaitActionName();
        else
            result = _10->turnAction;
    } else if (_10 != &sDefaultNpcActionAnimParam) {
        if (al::isEqualString(actionName, "Walk"))
            result = _10->walkAction;
        else if (al::isEqualString(actionName, "Talk"))
            result = _10->talkAction;
        else if (al::isEqualString(actionName, "TurnL90"))
            result = _10->turnL90Action;
        else if (al::isEqualString(actionName, "TurnR90"))
            result = _10->turnR90Action;
        else if (al::isEqualString(actionName, "Turn180"))
            result = _10->turn180Action;
        else if (al::isEqualString(actionName, "Give"))
            result = _10->giveAction;
    }

    return rs::makeNpcActionName(buffer, _8, result);
}

void TalkNpcActionAnimInfo::changeWaitActionName(const char* actionName,
                                                 const TalkNpcParam* param) {
    _38 = actionName;
    const char* effectiveWait = getWaitActionName();
    _60 = param->isInvalidChangeTurnAnimFromWait(effectiveWait);
    _61 = param->isInvalidChangeAllAnimFromWait(effectiveWait);
}

void TalkNpcActionAnimInfo::changeHackWaitActionName(const char* actionName,
                                                     const TalkNpcParam* param) {
    _40 = actionName;
    const char* effectiveWait = getWaitActionName();
    _60 = param->isInvalidChangeTurnAnimFromWait(effectiveWait);
    _61 = param->isInvalidChangeAllAnimFromWait(effectiveWait);
}

void TalkNpcActionAnimInfo::onHackWaitActionName(const TalkNpcParam* param) {
    _30 = true;
    const char* effectiveWait = getWaitActionName();
    _60 = param->isInvalidChangeTurnAnimFromWait(effectiveWait);
    _61 = param->isInvalidChangeAllAnimFromWait(effectiveWait);
}

void TalkNpcActionAnimInfo::offHackWaitActionName(const TalkNpcParam* param) {
    _30 = false;
    const char* effectiveWait = getWaitActionName();
    _60 = param->isInvalidChangeTurnAnimFromWait(effectiveWait);
    _61 = param->isInvalidChangeAllAnimFromWait(effectiveWait);
}

bool TalkNpcActionAnimInfo::changeWaitActionNameBySwitch(const char* actionName,
                                                        const TalkNpcParam* param) {
    al::StringTmp<64> prevAction("");
    convertActionName(&prevAction, getWaitActionName());

    _48 = actionName;
    const char* effectiveWait = getWaitActionName();
    _60 = param->isInvalidChangeTurnAnimFromWait(effectiveWait);
    _61 = param->isInvalidChangeAllAnimFromWait(effectiveWait);

    return al::isActionPlaying(_8, prevAction.cstr());
}

bool TalkNpcActionAnimInfo::resetWaitActionNameBySwitch(const TalkNpcParam* param) {
    al::StringTmp<64> prevAction("");
    convertActionName(&prevAction, getWaitActionName());

    _48 = nullptr;
    const char* effectiveWait = getWaitActionName();
    _60 = param->isInvalidChangeTurnAnimFromWait(effectiveWait);
    _61 = param->isInvalidChangeAllAnimFromWait(effectiveWait);

    return al::isActionPlaying(_8, prevAction.cstr());
}

bool TalkNpcActionAnimInfo::isSelectedInitWaitAction() const {
    return al::isEqualString(mWaitActionName, getWaitActionName());
}

const char* TalkNpcActionAnimInfo::getAnyRandomActionName() const {
    s32 index = al::getRandom(_50);
    if (index > _50)
        index = _50;
    return _58[index];
}
