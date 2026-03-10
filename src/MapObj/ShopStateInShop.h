#pragma once

class Shine;

namespace al {
class LayoutActor;
}  // namespace al

class ShopItemState {
public:
    ShopItemState();
    void init();
    void onBuyShine();
    void onBuyLifeUpItem();
    bool isBuyAlreadyShineBeforeGameClear(const Shine* shine) const;
    bool isBuyAlreadyShineAfterGameClear(const Shine* shine) const;
    bool isBuyAlreadyLifeUpItem(const al::LayoutActor* actor) const;

private:
    bool mIsBoughtLifeUp = false;
    bool mIsBoughtShine = false;
};

static_assert(sizeof(ShopItemState) == 0x2);
