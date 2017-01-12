#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"

#include "TArc/DataProcessing/DataWrappersProcessor.h"

#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
namespace TArc
{
BaseComponentValue::BaseComponentValue()
{
    thisValue = this;
}

void BaseComponentValue::Init(ReflectedPropertyModel* model_)
{
    model = model_;
}

QString BaseComponentValue::GetPropertyName() const
{
    Any fieldName = nodes.front()->field.key;
    const Type* nameType = fieldName.GetType();
    if (nameType == Type::Instance<int>())
    {
        return QString::number(fieldName.Cast<int>());
    }
    else if (nameType == Type::Instance<const char*>())
    {
        return QString(fieldName.Cast<const char*>());
    }

    return QString::fromStdString(fieldName.Cast<String>());
}

int32 BaseComponentValue::GetPropertiesNodeCount() const
{
    return static_cast<int32>(nodes.size());
}

std::shared_ptr<const PropertyNode> BaseComponentValue::GetPropertyNode(int32 index) const
{
    DVASSERT(static_cast<size_t>(index) < nodes.size());
    return nodes[static_cast<size_t>(index)];
}

void BaseComponentValue::AddPropertyNode(const std::shared_ptr<PropertyNode>& node)
{
    nodes.push_back(node);
}

void BaseComponentValue::RemovePropertyNode(const std::shared_ptr<PropertyNode>& node)
{
    auto iter = std::find(nodes.begin(), nodes.end(), node);
    if (iter == nodes.end())
    {
        return;
    }

    nodes.erase(iter);
}

void BaseComponentValue::RemovePropertyNodes()
{
    nodes.clear();
}

std::shared_ptr<ModifyExtension> BaseComponentValue::GetModifyInterface()
{
    return model->GetExtensionChain<ModifyExtension>();
}

DAVA::TArc::DataWrappersProcessor* BaseComponentValue::GetWrappersProcessor()
{
    return &model->wrappersProcessor;
}

DAVA::Reflection BaseComponentValue::GetReflection()
{
    return Reflection::Create(&thisValue);
}

DAVA_REFLECTION_IMPL(BaseComponentValue)
{
    ReflectionRegistrator<BaseComponentValue>::Begin()
    .End();
}
}
}