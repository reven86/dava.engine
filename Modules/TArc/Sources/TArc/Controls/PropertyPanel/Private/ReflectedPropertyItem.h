#pragma once

#include "Base/BaseTypes.h"

#include <QString>
#include <memory>

namespace DAVA
{
namespace TArc
{
class ReflectedPropertyModel;
class BaseComponentValue;
struct PropertyNode;

class ReflectedPropertyItem
{
public:
    ~ReflectedPropertyItem();

    ReflectedPropertyItem(const ReflectedPropertyItem& other) = delete;
    ReflectedPropertyItem(ReflectedPropertyItem&& other) = delete;
    ReflectedPropertyItem& operator=(const ReflectedPropertyItem& other) = delete;
    ReflectedPropertyItem& operator=(ReflectedPropertyItem&& other) = delete;

    int32 GetPropertyNodesCount() const;
    std::shared_ptr<const PropertyNode> GetPropertyNode(int32 index) const;

    QString GetPropertyName() const;

private:
    friend class ReflectedPropertyModel;
    ReflectedPropertyItem(ReflectedPropertyModel* model, std::unique_ptr<BaseComponentValue>&& value);
    ReflectedPropertyItem(ReflectedPropertyModel* model, ReflectedPropertyItem* parent, int32 position, std::unique_ptr<BaseComponentValue>&& value);
    ReflectedPropertyItem* CreateChild(std::unique_ptr<BaseComponentValue>&& value, int32 childPosition);

    int32 GetChildCount() const;
    ReflectedPropertyItem* GetChild(int32 index) const;
    void RemoveChild(int32 index);
    void RemoveChildren();

    void AddPropertyNode(const std::shared_ptr<PropertyNode>& node);
    void RemovePropertyNode(const std::shared_ptr<PropertyNode>& node);
    void RemovePropertyNodes();

private:
    ReflectedPropertyModel* model;
    ReflectedPropertyItem* parent = nullptr;
    int32 position = 0;
    Vector<std::unique_ptr<ReflectedPropertyItem>> children;
    std::unique_ptr<BaseComponentValue> value;
};
} // namespace TArc
} // namespace DAVA