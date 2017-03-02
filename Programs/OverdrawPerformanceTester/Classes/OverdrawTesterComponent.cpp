#include "OverdrawTesterComponent.h"

#include "OverdrawTesterRenderObject.h"
#include "Scene3D/Entity.h"
#include "Math/Matrix4.h"

namespace OverdrawPerformanceTester
{
const uint8 OverdrawTesterComonent::addOverdrawPercent = 10;

OverdrawTesterComonent::OverdrawTesterComonent(uint16 textureResolution_, uint8 overdrawScreenCount)
    : textureResolution(textureResolution_)
    , stepsCount(overdrawScreenCount * 100 / addOverdrawPercent)
{
    renderObject = new OverdrawTesterRenderObject(addOverdrawPercent, stepsCount, textureResolution);
    renderObject->SetWorldTransformPtr(&DAVA::Matrix4::IDENTITY);
}

OverdrawTesterComonent::~OverdrawTesterComonent()
{
    SafeRelease(renderObject);
}

DAVA::Component* OverdrawTesterComonent::Clone(DAVA::Entity* toEntity)
{
    OverdrawTesterComonent* newComponent = new OverdrawTesterComonent(textureResolution, (stepsCount * addOverdrawPercent) / 100);
    newComponent->SetEntity(toEntity);
    return newComponent;
}
}
