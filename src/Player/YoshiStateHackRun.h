#pragma once

class YoshiStateHackRun {
public:
    void invalidateTurn();
    void validateTurn();

private:
    struct TurnControl {
        unsigned char _0[0xba];
        bool turnInvalid;
    };

    unsigned char _0[0x68];
    TurnControl* mTurnControl;
};
