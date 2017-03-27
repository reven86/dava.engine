#include "TArc/Controls/PropertyPanel/Private/SubPropertiesExtensions.h"
#include "TArc/Controls/PropertyPanel/Private/kDComponentValue.h"

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
UnorderedSet<const Type*> subPropertyTypes;

void InitSubPropertyTypes()
{
    if (subPropertyTypes.empty())
    {
        subPropertyTypes.insert(Type::Instance<Vector2>());
        subPropertyTypes.insert(Type::Instance<Vector3>());
        subPropertyTypes.insert(Type::Instance<Vector4>());
        subPropertyTypes.insert(Type::Instance<Rect>());
        subPropertyTypes.insert(Type::Instance<AABBox3>());
        subPropertyTypes.insert(Type::Instance<Color>());
    }
}
}

void SubPropertyValueChildCreator::ExposeChildren(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const
{
    using namespace SubPropertiesExtensionsDetail;
    InitSubPropertyTypes();
    if (parent->propertyType == PropertyNode::RealProperty)
    {
        const Type* valueType = parent->field.ref.GetValueType()->Decay();
        if (subPropertyTypes.count(valueType) > 0)
        {
            return;
        }
    }

    ChildCreatorExtension::ExposeChildren(parent, children);
}

std::unique_ptr<BaseComponentValue> SubPropertyEditorCreator::GetEditor(const std::shared_ptr<const PropertyNode>& node) const
{
    using namespace SubPropertiesExtensionsDetail;
    InitSubPropertyTypes();

    const Type* valueType = node->field.ref.GetValueType()->Decay();
    if (subPropertyTypes.count(valueType) > 0)
    {
        if (valueType == Type::Instance<Vector2>())
        {
            return std::make_unique<kDComponentValue<Vector2, MultiDoubleSpinBox, float32>>();
        }
        else if (valueType == Type::Instance<Vector3>())
        {
            return std::make_unique<kDComponentValue<Vector3, MultiDoubleSpinBox, float32>>();
        }
        else if (valueType == Type::Instance<Vector4>())
        {
            return std::make_unique<kDComponentValue<Vector4, MultiDoubleSpinBox, float32>>();
        }
        else if (valueType == Type::Instance<Rect>())
        {
            return std::make_unique<kDComponentValue<Rect, MultiDoubleSpinBox, float32>>();
        }
        else if (valueType == Type::Instance<Color>())
        {
            return std::make_unique<kDComponentValue<Color, MultiDoubleSpinBox, float32>>();
        }
        else if (valueType == Type::Instance<AABBox3>())
        {
            return std::make_unique<kDComponentValue<AABBox3, MultiDoubleSpinBox, float32>>();
        }
    }

    return EditorComponentExtension::GetEditor(node);
}

} // namespace TArc
} // namespace DAVA