#pragma once

#include "Library/Layout/LayoutActor.h"

class ShopLayoutFooter : public al::LayoutActor {
public:
    ShopLayoutFooter();

    void appear() override;

    void reset();
    void end();
    void tryStartDecide();
    void tryStartCommon(const char* messageId);
    void tryStartSelect();

    void exeWait();
    void exeFadeIn();
    void exeFadeInAfterFadeOut();
    void exeFadeOut();
    void exeEnd();

private:
    const char* mMessageId = "";
};

static_assert(sizeof(ShopLayoutFooter) == 0x138);
