#include "TArc/Controls/PropertyPanel/KeyedArchiveChildCreator.h"
#include "TArc/Utils/ReflectionHelpers.h"

#include <FileSystem/KeyedArchive.h>
#include <Logger/Logger.h>
#include <Base/Type.h>

namespace DAVA
{
namespace TArc
{
void KeyedArchiveChildCreator::ExposeChildren(const std::shared_ptr<const PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const
{
    if (parent->field.ref.GetValueType() == Type::Instance<KeyedArchive*>())
    {
        Reflection objectMapField = parent->field.ref.GetField("objectMap");
        if (objectMapField.IsValid())
        {
            ForEachField(objectMapField, [this, &children](Reflection::Field&& f)
                         {
                             children.push_back(allocator->CreatePropertyNode(std::move(f)));
                         });
        }
        else
        {
            Logger::FrameworkDebug("Can't find \"objectMap\" field in KeyedArchive");
        }

        return;
    }

    ChildCreatorExtension::ExposeChildren(parent, children);
}
} // namespace TArc
} // namespace DAVA
