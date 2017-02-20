#include "OverdrawTesterSystem.h"

#include "OverdrawTesterComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderObject.h"
#include "OverdrawTesterRenderObject.h"

namespace OverdrawPerformanceTester
{

OverdrawTesterSystem::OverdrawTesterSystem(DAVA::Scene* scene) : SceneSystem(scene)
{
}

OverdrawTesterSystem::~OverdrawTesterSystem()
{

}

void OverdrawTesterSystem::AddEntity(DAVA::Entity* entity)
{
    OverdrawTesterComonent* comp = static_cast<OverdrawTesterComonent*>(entity->GetComponent(OverdrawTesterComonent::OVERDRAW_TESTER_COMPONENT));
    if (comp != nullptr)
        GetScene()->GetRenderSystem()->RenderPermanent(comp->GetRenderObject());
}

void OverdrawTesterSystem::RemoveEntity(DAVA::Entity* entity)
{
    OverdrawTesterComonent* comp = static_cast<OverdrawTesterComonent*>(entity->GetComponent(OverdrawTesterComonent::OVERDRAW_TESTER_COMPONENT));
    if (comp != nullptr)
        GetScene()->GetRenderSystem()->RemoveFromRender(comp->GetRenderObject());
}

}
