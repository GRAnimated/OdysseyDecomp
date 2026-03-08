#include "Layout/ShopLayoutFooter.h"

#include "Library/Base/StringUtil.h"
#include "Library/Layout/LayoutActionFunction.h"
#include "Library/Layout/LayoutActorUtil.h"
#include "Library/Message/MessageHolder.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

namespace {
NERVE_IMPL(ShopLayoutFooter, Wait);
NERVE_IMPL(ShopLayoutFooter, FadeIn);
NERVE_IMPL(ShopLayoutFooter, End);
NERVE_IMPL(ShopLayoutFooter, FadeInAfterFadeOut);
NERVE_IMPL(ShopLayoutFooter, FadeOut);

NERVES_MAKE_NOSTRUCT(ShopLayoutFooter, End, FadeOut);
NERVES_MAKE_STRUCT(ShopLayoutFooter, Wait, FadeIn, FadeInAfterFadeOut);
}  // namespace

ShopLayoutFooter::ShopLayoutFooter() : al::LayoutActor("[ショップ]フッター") {
    initNerve(&NrvShopLayoutFooter.Wait, 0);
}

void ShopLayoutFooter::appear() {}

void ShopLayoutFooter::reset() {
    al::setNerve(this, &End);
}

void ShopLayoutFooter::end() {
    al::setNerve(this, &FadeOut);
}

void ShopLayoutFooter::tryStartDecide() {
    if (al::isEqualString(mMessageId, "Footer_Decide"))
        return;
    mMessageId = "Footer_Decide";
    al::LayoutActor::appear();
    if (al::isNerve(this, &NrvShopLayoutFooter.Wait))
        al::setNerve(this, &NrvShopLayoutFooter.FadeInAfterFadeOut);
    else
        al::setNerve(this, &NrvShopLayoutFooter.FadeIn);
}

void ShopLayoutFooter::tryStartCommon(const char* messageId) {
    if (al::isEqualString(mMessageId, messageId))
        return;
    mMessageId = messageId;
    al::LayoutActor::appear();
    if (al::isNerve(this, &NrvShopLayoutFooter.Wait))
        al::setNerve(this, &NrvShopLayoutFooter.FadeInAfterFadeOut);
    else
        al::setNerve(this, &NrvShopLayoutFooter.FadeIn);
}

void ShopLayoutFooter::tryStartSelect() {
    if (al::isEqualString(mMessageId, "Footer_Select"))
        return;
    mMessageId = "Footer_Select";
    al::LayoutActor::appear();
    if (al::isNerve(this, &NrvShopLayoutFooter.Wait))
        al::setNerve(this, &NrvShopLayoutFooter.FadeInAfterFadeOut);
    else
        al::setNerve(this, &NrvShopLayoutFooter.FadeIn);
}

void ShopLayoutFooter::exeWait() {
    if (al::isFirstStep(this)) {
        al::IUseLayout* layout = this;
        const char16* message = al::getSystemMessageString(this, "ShopMessage", mMessageId);
        al::setPaneString(layout, "TxtGuide", message);
    }
}

void ShopLayoutFooter::exeFadeIn() {
    if (al::isFirstStep(this)) {
        al::IUseLayout* layout = this;
        const char16* message = al::getSystemMessageString(this, "ShopMessage", mMessageId);
        al::setPaneString(layout, "TxtGuide", message);
        al::startAction(this, "FadeIn");
    }
    if (al::isActionEnd(this))
        al::setNerve(this, &NrvShopLayoutFooter.Wait);
}

void ShopLayoutFooter::exeFadeInAfterFadeOut() {
    if (al::isFirstStep(this))
        al::startAction(this, "FadeOut");
    if (al::isActionEnd(this))
        al::setNerve(this, &NrvShopLayoutFooter.FadeIn);
}

void ShopLayoutFooter::exeFadeOut() {
    if (al::isFirstStep(this))
        al::startAction(this, "FadeOut");
    if (al::isActionEnd(this))
        al::setNerve(this, &End);
}

void ShopLayoutFooter::exeEnd() {
    if (al::isFirstStep(this))
        mMessageId = "";
}
