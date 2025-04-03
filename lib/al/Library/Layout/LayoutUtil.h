#pragma once

namespace agl {
class RenderBuffer;

}  // namespace agl

namespace al {
class AudioDirector;
class LayoutInitInfo;
class GamePadSystem;
class MessageSystem;
class LayoutKit;
class LayoutSystem;
class SceneObjHolder;

void initLayoutInitInfo(LayoutInitInfo*, const LayoutKit*, SceneObjHolder*, const AudioDirector*,
                        const LayoutSystem*, const MessageSystem*, const GamePadSystem*);
void setRenderBuffer(LayoutKit*, const agl::RenderBuffer*);
void executeUpdate(al::LayoutKit*);
void executeUpdateList(al::LayoutKit*, const char*, const char*);
void executeUpdateEffect(al::LayoutKit*);
void executeDraw(const al::LayoutKit*, const char*);
void executeDrawEffect(const al::LayoutKit*);

}  // namespace al
