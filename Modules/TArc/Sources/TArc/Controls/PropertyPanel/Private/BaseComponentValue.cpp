#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "TArc/Controls/PropertyPanel/ReflectedPropertyModel.h"

#include "TArc/DataProcessing/QtReflectionBridge.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"

#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
namespace TArc
{
BaseComponentValue::BaseComponentValue()
    : thisValue(this)
{
}

BaseComponentValue::~BaseComponentValue()
{
    delete qtReflected;
}

void BaseComponentValue::Init(ReflectedPropertyModel* model_)
{
    model = model_;
    QtReflectionBridge* bridge = model->reflectionBridge;
    DVASSERT(bridge != nullptr);

    qtReflected = bridge->CreateQtReflected(model->wrappersProcessor.CreateWrapper(DAVA::MakeFunction(this, &BaseComponentValue::GetData), nullptr), nullptr);
    qtReflected->Init();
}

DAVA::TArc::QtReflected* BaseComponentValue::GetValueObject()
{
    return qtReflected;
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

DAVA::Reflection BaseComponentValue::GetData(const DataContext* /*ctx*/)
{
    return Reflection::Create(this);
}

DAVA_REFLECTION_IMPL(BaseComponentValue)
{
    ReflectionRegistrator<BaseComponentValue>::Begin()
    .End();
}
}
}