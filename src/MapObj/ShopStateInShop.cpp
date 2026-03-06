#include "MapObj/ShopStateInShop.h"

#include "Library/Layout/LayoutActor.h"

#include "Item/Shine.h"
#include "System/GameDataFunction.h"
#include "System/GameDataHolderAccessor.h"

ShopItemState::ShopItemState() = default;

void ShopItemState::init() {
    mIsBoughtLifeUp = false;
    mIsBoughtShine = false;
}

void ShopItemState::onBuyShine() {
    mIsBoughtShine = true;
}

void ShopItemState::onBuyLifeUpItem() {
    mIsBoughtLifeUp = true;
}

// NON_MATCHING: branch layout differs (shared tail block placed differently)
bool ShopItemState::isBuyAlreadyShineBeforeGameClear(const Shine* shine) const {
    if (GameDataFunction::isGameClear(shine))
        return false;
    if (mIsBoughtShine)
        return true;
    return GameDataFunction::isGotShine(shine);
}

// NON_MATCHING: branch layout differs (shared tail block placed differently)
bool ShopItemState::isBuyAlreadyShineAfterGameClear(const Shine* shine) const {
    if (!GameDataFunction::isGameClear(shine))
        return false;
    if (mIsBoughtShine)
        return true;
    return GameDataFunction::isGotShine(shine);
}

bool ShopItemState::isBuyAlreadyLifeUpItem(const al::LayoutActor* actor) const {
    return GameDataFunction::isPlayerHitPointMaxWithItem(actor) || mIsBoughtLifeUp;
}
