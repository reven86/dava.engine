#pragma once

#include "PropertyModelExtensions.h"

#include "Reflection/Reflection.h"
#include "Base/BaseTypes.h"

class QQmlComponent;
class QQmlEngine;

namespace DAVA
{
namespace TArc
{
class ReflectedPropertyModel;
class QtReflected;
class DataContext;
class DataWrappersProcessor;
struct PropertyNode;

class BaseComponentValue : public ReflectionBase
{
public:
    void Init(ReflectedPropertyModel* model);

    int32 GetPropertiesNodeCount() const;
    std::shared_ptr<const PropertyNode> GetPropertyNode(int32 index) const;

protected:
    friend class ReflectedPropertyItem;

    void AddPropertyNode(const std::shared_ptr<PropertyNode>& node);
    void RemovePropertyNode(const std::shared_ptr<PropertyNode>& node);
    void RemovePropertyNodes();

    Vector<std::shared_ptr<PropertyNode>> nodes;
    std::shared_ptr<ModifyExtension> GetModifyInterface();

private:
    ReflectedPropertyModel* model = nullptr;

    DAVA_VIRTUAL_REFLECTION(BaseComponentValue);
};
}
}