#include "TArc/Controls/PropertyPanel/Private/SubPropertiesExtensions.h"
#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

#include <Math/Vector.h>
#include <Math/Rect.h>
#include <Math/AABBox3.h>
#include <Math/Color.h>
#include <Base/BaseTypes.h>

namespace DAVA
{
namespace TArc
{
namespace SubPropertiesExtensionsDetail
{
const UnorderedSet<const Type*> subPropertyTypes =
{
  Type::Instance<Vector2>(),
  Type::Instance<Vector3>(),
  Type::Instance<Vector4>(),
  Type::Instance<Rect>(),
  Type::Instance<AABBox3>(),
  Type::Instance<Color>()
};
}

void SubPropertyValueChildCreator::ExposeChildren(const std::shared_ptr<const PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const
{
    using namespace SubPropertiesExtensionsDetail;
    if (parent->propertyType == PropertyNode::RealProperty)
    {
        if (subPropertyTypes.count(parent->field.ref.GetValueType()) > 0)
        {
            return;
        }
    }

    ChildCreatorExtension::ExposeChildren(parent, children);
}

std::unique_ptr<BaseComponentValue> SubPropertyEditorCreator::GetEditor(const std::shared_ptr<const PropertyNode>& node) const
{
    using namespace SubPropertiesExtensionsDetail;
    if (subPropertyTypes.count(node->field.ref.GetValueType()) > 0)
    {
    }

    return EditorComponentExtension::GetEditor(node);
}

} // namespace TArc
} // namespace DAVA