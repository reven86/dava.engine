#pragma once

#include "QtTools/Utils/QtConnections.h"

class SceneEditor2;
class ActiveSceneHolder
{
public:
    SceneEditor2* GetScene() const;
    static void Init();
    static void Deinit();
};
