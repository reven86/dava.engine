#include "SetPropertyValueCommand.h"

#include "Base/Introspection.h"

namespace DAVA
{
SetPropertyValueCommand::SetPropertyValueCommand(const ObjectHandle& object_, const InspMember* property_, VariantType newValue_)
    : object(object_)
    , property(property_)
    , newValue(newValue_)
{
    DVASSERT(object.IsValid() == true);
    DVASSERT(object.GetIntrospection() != nullptr);
    DVASSERT(object.GetIntrospection()->Member(property->Name()) != nullptr);

    const DAVA::MetaInfo* propertyType = property->Type();
    if (newValue.Meta() != propertyType)
    {
        newValue = VariantType::Convert(newValue, propertyType);
        DVASSERT(newValue.Meta() == propertyType);
    }
}

void SetPropertyValueCommand::Execute()
{
    oldValue = property->Value(object.GetObjectPointer());
    Redo();
}

void SetPropertyValueCommand::Redo()
{
    property->SetValue(object.GetObjectPointer(), newValue);
}

void SetPropertyValueCommand::Undo()
{
    property->SetValue(object.GetObjectPointer(), oldValue);
}
}
