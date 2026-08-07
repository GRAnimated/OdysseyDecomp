#pragma once

class YoshiJudgeWallCling {
public:
    void reset();
    bool judge() const;

private:
    unsigned char _0[0x30];
    bool mIsDamageWall;
    bool mIsJudge;
};
