#pragma once

class PlayerHackKeeper;

class IUsePlayerHack {
public:
    virtual PlayerHackKeeper* getPlayerHackKeeper() const = 0;
};

static_assert(sizeof(IUsePlayerHack) == 0x8);
