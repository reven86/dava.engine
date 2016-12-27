#include "TArc/Controls/PropertyPanel/ReflectedPropertyItem.h"
#include "Tarc/Controls/PropertyPanel/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

#include "Debug/DVAssert.h"

#include <QPointer>
#include <QQmlEngine>

namespace DAVA
{
namespace TArc
{
ReflectedPropertyItem::ReflectedPropertyItem(ReflectedPropertyModel* model_, std::unique_ptr<BaseComponentValue>&& value_)
    : model(model_)
    , value(std::move(value_))
{
}

ReflectedPropertyItem::ReflectedPropertyItem(ReflectedPropertyModel* model_, ReflectedPropertyItem* parent_, int32 position_, std::unique_ptr<BaseComponentValue>&& value_)
    : model(model_)
    , parent(parent_)
    , position(position_)
    , value(std::move(value_))
{
}

int32 ReflectedPropertyItem::GetPropertyNodesCount() const
{
    return value->GetPropertiesNodeCount();
}

std::shared_ptr<const PropertyNode> ReflectedPropertyItem::GetPropertyNode(int32 index) const
{
    return value->GetPropertyNode(index);
}

QString ReflectedPropertyItem::GetPropertyName() const
{
    return QString::fromStdString(value->GetPropertyNode(0)->fieldName.Cast<String>());
}

QQmlComponent* ReflectedPropertyItem::GetComponent() const
{
    QPointer<QQmlEngine> engine = model->GetQmlEngine();
    if (engine.isNull())
    {
        return nullptr;
    }
    return value->GetComponent(engine.data());
}

QtReflected* ReflectedPropertyItem::GetModel() const
{
    return value->GetValueObject();
}

ReflectedPropertyItem* ReflectedPropertyItem::CreateChild(std::unique_ptr<BaseComponentValue>&& value)
{
    int32 position = static_cast<int32>(children.size());
    children.emplace_back(new ReflectedPropertyItem(model, this, position, std::move(value)));
    return children.back().get();
}

int32 ReflectedPropertyItem::GetChildCount() const
{
    return static_cast<int32>(children.size());
}

ReflectedPropertyItem* ReflectedPropertyItem::GetChild(int32 index) const
{
    DVASSERT(index < GetChildCount());
    return children[index].get();
}

void ReflectedPropertyItem::RemoveChild(int32 index)
{
    DVASSERT(index < GetChildCount());
    auto childIter = children.begin() + index;
    for (auto iter = childIter + 1; iter != children.end(); ++iter)
    {
        --((*iter)->position);
    }
    children.erase(childIter);
}

void ReflectedPropertyItem::RemoveChildren()
{
    children.clear();
}

void ReflectedPropertyItem::AddPropertyNode(const std::shared_ptr<PropertyNode>& node)
{
    value->AddPropertyNode(node);
}

void ReflectedPropertyItem::RemovePropertyNode(const std::shared_ptr<PropertyNode>& node)
{
    value->RemovePropertyNode(node);
}

void ReflectedPropertyItem::RemovePropertyNodes()
{
    value->RemovePropertyNodes();
}

} // namespace TArc
} // namespace DAVA