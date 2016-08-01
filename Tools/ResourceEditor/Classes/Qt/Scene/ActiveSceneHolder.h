#pragma once

#include "QtTools/Utils/QtConnections.h"

class SceneEditor2;
class ActiveSceneHolder
{
public:
    ActiveSceneHolder();

    SceneEditor2* GetScene() const;

private:
    SceneEditor2* activeScene = nullptr;
    QtConnections connections;
};
