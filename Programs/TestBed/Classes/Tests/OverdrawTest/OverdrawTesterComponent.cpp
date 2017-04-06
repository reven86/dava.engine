#include "OverdrawTesterComponent.h"

#include "OverdrawTesterRenderObject.h"
#include "Scene3D/Entity.h"
#include "Math/Matrix4.h"

namespace OverdrawPerformanceTester
{
using DAVA::uint8;
using DAVA::uint16;

const uint8 OverdrawTesterComponent::addOverdrawPercent = 10;

OverdrawTesterComponent::OverdrawTesterComponent(uint16 textureResolution_, uint8 overdrawScreenCount)
    : textureResolution(textureResolution_)
    , stepsCount(overdrawScreenCount * 100 / addOverdrawPercent)
{
    renderObject = new OverdrawTesterRenderObject(addOverdrawPercent, stepsCount, textureResolution);
    renderObject->SetWorldTransformPtr(&DAVA::Matrix4::IDENTITY);
}

OverdrawTesterComponent::~OverdrawTesterComponent()
{
    SafeRelease(renderObject);
}

DAVA::Component* OverdrawTesterComponent::Clone(DAVA::Entity* toEntity)
{
    OverdrawTesterComponent* newComponent = new OverdrawTesterComponent(textureResolution, (stepsCount * addOverdrawPercent) / 100);
    newComponent->SetEntity(toEntity);
    return newComponent;
}
}
