#pragma once

#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>
#include <Functional/Functional.h>
#include <FileSystem/VariantType.h>
#include <UI/Components/UIComponent.h>

class ControlInformation
{
public:
    virtual ~ControlInformation() = default;

    virtual DAVA::FastName GetName() const = 0;
    virtual DAVA::FastName GetPrototype() const = 0;
    virtual DAVA::String GetPrototypePackagePath() const = 0;

    virtual bool HasComponent(const DAVA::Type* componentType) const = 0;

    virtual void VisitParent(const DAVA::Function<void(const ControlInformation*)>& visitor) const = 0;
    virtual void VisitChildren(const DAVA::Function<void(const ControlInformation*)>& visitor) const = 0;

    virtual DAVA::Any GetControlPropertyValue(const DAVA::ReflectedStructure::Field& member) const = 0;
};
