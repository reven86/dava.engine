#pragma once

#include "Entity/SceneSystem.h"

namespace DAVA
{
class Scene;
}

namespace OverdrawPerformanceTester
{
class OverdrawTesterSystem : public DAVA::SceneSystem
{
public:
    OverdrawTesterSystem(DAVA::Scene* scene);
    ~OverdrawTesterSystem();

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

private:

};
}