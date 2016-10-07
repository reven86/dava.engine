#include "ActiveSceneHolder.h"
#include "SceneSignals.h"

#include "TArcUtils/QtConnections.h"

namespace ActiveSceneHolderDetails
{
class ActiveSceneHolderImpl : public DAVA::Singleton<ActiveSceneHolderImpl>
{
public:
    ActiveSceneHolderImpl()
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

    SceneEditor2* activeScene = nullptr;
    QtConnections connections;
};
}

SceneEditor2* ActiveSceneHolder::GetScene() const
{
    return ActiveSceneHolderDetails::ActiveSceneHolderImpl::Instance()->activeScene;
}

void ActiveSceneHolder::Init()
{
    new ActiveSceneHolderDetails::ActiveSceneHolderImpl();
}

void ActiveSceneHolder::Deinit()
{
    ActiveSceneHolderDetails::ActiveSceneHolderImpl::Instance()->Release();
}
