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

    OverdrawTesterComonent(DAVA::uint16 textureResolution_, DAVA::uint8 overdrawScreenCount);
    ~OverdrawTesterComonent();

    inline OverdrawTesterRenderObject* GetRenderObject() const;
    Component* Clone(DAVA::Entity* toEntity) override;

    inline uint32 GetStepsCount() const;
    inline float32 GetStepOverdraw() const;

private:
    OverdrawTesterRenderObject* renderObject;
    DAVA::uint32 stepsCount = 10;
    DAVA::uint16 textureResolution = 2048;

    static const DAVA::uint8 addOverdrawPercent;

public:
    INTROSPECTION_EXTEND(OverdrawTesterComonent, Component,
                         NULL);
};

OverdrawTesterRenderObject* OverdrawTesterComonent::GetRenderObject() const
{
    return renderObject;
}

DAVA::uint32 OverdrawTesterComonent::GetStepsCount() const
{
    return stepsCount;
}

DAVA::float32 OverdrawTesterComonent::GetStepOverdraw() const
{
    return addOverdrawPercent;
}
}
