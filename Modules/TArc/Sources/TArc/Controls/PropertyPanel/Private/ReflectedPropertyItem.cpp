#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyItem.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

#include "Debug/DVAssert.h"

#include <QPointer>

namespace DAVA
{
namespace TArc
{
ReflectedPropertyItem::~ReflectedPropertyItem() = default;

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

std::shared_ptr<PropertyNode> ReflectedPropertyItem::GetPropertyNode(int32 index) const
{
    return value->GetPropertyNode(index);
}

QString ReflectedPropertyItem::GetPropertyName() const
{
    return value->GetPropertyName();
}

FastName ReflectedPropertyItem::GetID() const
{
    return value->GetID();
}

bool ReflectedPropertyItem::IsFavorite() const
{
    return isFavorite;
}

void ReflectedPropertyItem::SetFavorite(bool isFavorite_)
{
    isFavorite = isFavorite_;
}

bool ReflectedPropertyItem::IsFavorited() const
{
    return isFavorited;
}

void ReflectedPropertyItem::SetFavorited(bool isFavorited_)
{
    isFavorited = isFavorited_;
}

ReflectedPropertyItem* ReflectedPropertyItem::CreateChild(std::unique_ptr<BaseComponentValue>&& value, int32 childPosition, int32 sortKey)
{
    int32 position = static_cast<int32>(children.size());
    ReflectedPropertyItem* item = nullptr;
    if (position < childPosition)
    {
        children.emplace_back(new ReflectedPropertyItem(model, this, position, std::move(value)));
        item = children.back().get();
    }
    else
    {
        auto iter = children.begin() + childPosition;
        iter = children.insert(iter, std::unique_ptr<ReflectedPropertyItem>(new ReflectedPropertyItem(model, this, childPosition, std::move(value))));
        for (auto tailIter = iter + 1; tailIter != children.end(); ++tailIter)
        {
            (*tailIter)->position++;
        }

        item = iter->get();
    }

    item->sortKey = sortKey;
    return item;
}

int32 ReflectedPropertyItem::LookupChildPosition(int32 sortKey)
{
    DVASSERT(sortKey != PropertyNode::InvalidSortKey);
    int32 childPosition = static_cast<int32>(children.size());
    for (size_t i = 0; i < children.size(); ++i)
    {
        ReflectedPropertyItem* child = children[i].get();
        if (sortKey < child->sortKey)
        {
            childPosition = static_cast<int32>(i);
            break;
        }
    }

    return childPosition;
}

int32 ReflectedPropertyItem::GetChildCount() const
{
    return static_cast<int32>(children.size());
}

ReflectedPropertyItem* ReflectedPropertyItem::GetChild(int32 index) const
{
    if (index < children.size())
        return children[index].get();
    return nullptr;
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

void ReflectedPropertyItem::AddPropertyNode(const std::shared_ptr<PropertyNode>& node, const FastName& id)
{
    value->AddPropertyNode(node, id);
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
