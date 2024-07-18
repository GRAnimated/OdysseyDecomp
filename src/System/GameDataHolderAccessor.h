#pragma once

namespace al {
class IUseSceneObjHolder;
class SceneObjHolder;
class LiveActor;
}  // namespace al
class GameDataHolder;

class GameDataHolderAccessor {
public:
    GameDataHolderAccessor(const al::IUseSceneObjHolder*);
    GameDataHolderAccessor(const al::SceneObjHolder*);

    GameDataHolderAccessor(GameDataHolder* holder) : mHolder(holder) {}

    GameDataHolder* getHolder() const { return mHolder; }

private:
    GameDataHolder* mHolder;
};

namespace rs {
bool isInvalidChangeStage(const al::LiveActor*);
bool isKidsMode(const al::IUseSceneObjHolder*);
}  // namespace rs
