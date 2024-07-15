#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
class ActorInitInfo;
}  // namespace al

class ItemGenerator {
public:
    ItemGenerator();
    ItemGenerator(al::LiveActor*, const al::ActorInitInfo&);

    void initLinkShine(al::LiveActor*, const al::ActorInitInfo&, bool);
    void initNoLinkShine(al::LiveActor*, const al::ActorInitInfo&, bool);
    void initHintPhotoShine(al::LiveActor*, const al::ActorInitInfo&);
    void createShineEffectInsideObject(const al::ActorInitInfo&);
    void tryUpdateHintTransIfExistShine();
    void tryUpdateHintTransIfExistShine(const sead::Vector3f&);
    bool isEnableGenerateByCount(s32) const;
    void generate(const sead::Vector3f&, const sead::Quatf&);
    void generate(const sead::Vector3f&, const sead::Vector3f&);
    void tryGenerate(const sead::Vector3f&, const sead::Quatf&, s32);
    void tryGenerate(const sead::Vector3f&, const sead::Vector3f&, s32);
    bool isNone() const;
    bool isShine() const;
    bool isLifeUp() const;
    bool isLifeMaxUp() const;
    bool isCoin() const;
    bool isCoinBlow() const;
    bool isCoinStackBound() const;
    bool isKuriboMini3() const;
    bool isKuriboMini8() const;

private:
    void* filler[5];
};
