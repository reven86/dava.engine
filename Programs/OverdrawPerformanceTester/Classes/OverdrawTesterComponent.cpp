#include "OverdrawTesterComponent.h"

#include "OverdrawTesterRenderObject.h"
#include "Scene3D/Entity.h"

namespace OverdrawPerformanceTester
{

OverdrawTesterComonent::OverdrawTesterComonent()
{
    renderObject = new OverdrawTesterRenderObject();
}

OverdrawTesterComonent::~OverdrawTesterComonent()
{
    SafeRelease(renderObject);
}

DAVA::Component* OverdrawTesterComonent::Clone(DAVA::Entity* toEntity)
{
    OverdrawTesterComonent* newComponent = new OverdrawTesterComonent();
    newComponent->SetEntity(toEntity);
    return newComponent;
}

}
