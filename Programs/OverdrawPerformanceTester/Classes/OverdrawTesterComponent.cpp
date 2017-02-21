#include "OverdrawTesterComponent.h"

#include "OverdrawTesterRenderObject.h"
#include "Scene3D/Entity.h"
#include "Math/Matrix4.h"

namespace OverdrawPerformanceTester
{

OverdrawTesterComonent::OverdrawTesterComonent()
{
    renderObject = new OverdrawTesterRenderObject(addOverdrawPercent);
    renderObject->SetWorldTransformPtr(&DAVA::Matrix4::IDENTITY);
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
