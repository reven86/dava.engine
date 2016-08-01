#include "ActiveSceneHolder.h"
#include "SceneSignals.h"

#include "Debug\DVAssert.h"

ActiveSceneHolder::ActiveSceneHolder()
{
    SceneSignals* dispatcher = SceneSignals::Instance();
    connections.AddConnection(dispatcher, &SceneSignals::Activated, [this](SceneEditor2* scene)
                              {
                                  activeScene = scene;
                              });

    connections.AddConnection(dispatcher, &SceneSignals::Deactivated, [this](SceneEditor2* scene)
                              {
                                  activeScene = nullptr;
                              });
}

SceneEditor2* ActiveSceneHolder::GetScene() const
{
    return activeScene;
}
