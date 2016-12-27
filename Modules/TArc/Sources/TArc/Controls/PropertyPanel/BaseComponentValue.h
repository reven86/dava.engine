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
class QtReflected;
class QtReflectionBridge;
class DataContext;
class DataWrappersProcessor;
struct PropertyNode;

class BaseComponentValue : public ReflectionBase
{
public:
    BaseComponentValue();
    virtual ~BaseComponentValue();

    virtual QQmlComponent* GetComponent(QQmlEngine* engine) const = 0;

    void Init(DataWrappersProcessor* wrappersProcessor, QtReflectionBridge* reflectionBridge);
    QtReflected* GetValueObject();

    int32 GetPropertiesNodeCount() const;
    std::shared_ptr<const PropertyNode> GetPropertyNode(int32 index) const;

protected:
    friend class ReflectedPropertyItem;

    void AddPropertyNode(const std::shared_ptr<PropertyNode>& node);
    void RemovePropertyNode(const std::shared_ptr<PropertyNode>& node);
    void RemovePropertyNodes();

    Vector<std::shared_ptr<PropertyNode>> nodes;

private:
    Reflection GetData(const DataContext* ctx);
    BaseComponentValue* thisValue = nullptr;
    QtReflected* qtReflected = nullptr;

    DAVA_VIRTUAL_REFLECTION(BaseComponentValue);
};
}
}