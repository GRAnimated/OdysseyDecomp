#include "Layout/ProjectReplaceTagProcessor.h"

#include <prim/seadSafeString.h>

#include "Library/Base/StringUtil.h"
#include "Library/Message/CustomTagProcessor.h"
#include "Library/Message/MessageHolder.h"
#include "Library/Scene/IUseSceneObjHolder.h"

#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"

ProjectReplaceTagProcessor::ProjectReplaceTagProcessor(const al::IUseSceneObjHolder* holder)
    : mSceneObjHolder(holder) {}

// NON_MATCHING: stack frame is 0xc0 in target vs 0xb0 here; compiler allocates separate stack
// slots for accessor temporaries in Shine else-branch and CoinCollect block instead of overlapping
u32 ProjectReplaceTagProcessor::replaceProjectTag(char16* out, const al::MessageTag& tag,
                                                  const al::IUseMessageSystem* msgSys) const {
    const char* tagName =
        al::getMessageTagName(msgSys, (s32)(u16)tag.getData()[1], (s32)(u16)tag.getData()[2]);
    const char16* message = nullptr;

    if (al::isEqualString(tagName, "ShineIconCurrentWorld")) {
        sead::FixedSafeString<64> iconName;
        bool isGameClear = false;
        GameDataFunction::findUnlockShineNum(&isGameClear, GameDataHolderAccessor(mSceneObjHolder));
        if (isGameClear)
            iconName.format("ShineIcon_All");
        else {
            s32 worldId =
                GameDataFunction::getCurrentWorldId(GameDataHolderAccessor(mSceneObjHolder));
            const char* developName = GameDataFunction::getWorldDevelopName(
                GameDataHolderAccessor(mSceneObjHolder), worldId);
            iconName.format("ShineIcon_%s", developName);
        }
        message = al::getSystemMessageString(msgSys, "IconTag", iconName.cstr());
    } else if (al::isEqualString(tagName, "CoinCollectIconCurrentWorld")) {
        sead::FixedSafeString<64> iconName;
        s32 worldId = GameDataFunction::getCurrentWorldId(GameDataHolderAccessor(mSceneObjHolder));
        const char* developName =
            GameDataFunction::getWorldDevelopName(GameDataHolderAccessor(mSceneObjHolder), worldId);
        iconName.format("CoinCollectIcon_%s", developName);
        message = al::getSystemMessageString(msgSys, "IconTag", iconName.cstr());
    } else if (al::isEqualSubString(tagName, "PadStyle")) {
        bool is2P = al::isEqualSubString(tagName, "PadStyle2P");
        u32 padStyleType = al::CustomTagProcessor::getPadStyleType(is2P);
        message = al::CustomTagProcessor::getPadStyleMessage(msgSys, tag.getData(), padStyleType);
    } else if (al::isEqualSubString(tagName, "PadPair")) {
        u32 padPairType = al::CustomTagProcessor::getPadPairType();
        message = al::CustomTagProcessor::getPadPairMessage(msgSys, tag.getData(), padPairType);
    } else {
        return 0;
    }

    u32 size = al::calcMessageSizeWithoutNullCharacter(message, nullptr);
    memcpy(out, message, 2 * size);
    return size;
}
