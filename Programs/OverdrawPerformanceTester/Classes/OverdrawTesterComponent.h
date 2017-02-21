#pragma once

#include "Base/BaseTypes.h"
#include "Entity/Component.h"

namespace DAVA
{
class Entity;
};

namespace OverdrawPerformanceTester
{
using DAVA::uint32;
using DAVA::float32;

class OverdrawTesterRenderObject;

class OverdrawTesterComonent : public DAVA::Component
{
public:
    enum eType
    {
        OVERDRAW_TESTER_COMPONENT = DAVA::Component::eType::FIRST_USER_DEFINED_COMPONENT + 1
    };
    IMPLEMENT_COMPONENT_TYPE(OVERDRAW_TESTER_COMPONENT);

    OverdrawTesterComonent();
    ~OverdrawTesterComonent();
    
    inline OverdrawTesterRenderObject* GetRenderObject() const;
    Component* Clone(DAVA::Entity* toEntity) override;

private:
    OverdrawTesterRenderObject* renderObject;
    float32 addOverdrawPercent = 10.0f;
    uint32 stepsCount = 100;

public:
    INTROSPECTION_EXTEND(OverdrawTesterComonent, Component,
        NULL);
};

OverdrawTesterRenderObject* OverdrawTesterComonent::GetRenderObject() const
{
    return renderObject;
}
}
