#include "TArc/Controls/PropertyPanel/DefaultPropertyModelExtensions.hpp"
#include "TArc/Controls/PropertyPanel/Private/ObjectsPool.hpp"
#include "TArc/Controls/PropertyPanel/ReflectedPropertyModel.hpp"

#include "Debug/DVAssert.h"

namespace DAVA
{
namespace TArc
{
namespace DPMEDetails
{
/*struct MaxMinRangePair
{
    MaxMinRangePair(const Any & min, const Any& max)
        : minValue_(min)
        , maxValue_(max)
    {
    }

    Any minValue_;
    Any maxValue_;
};

MaxMinRangePair getValuePair(const Type* type)
{
    static const TypeId int8Type = TypeId::getType<int8_t>();
    static const TypeId int16Type = TypeId::getType<int16_t>();
    static const TypeId int32Type = TypeId::getType<int32_t>();
    static const TypeId int64Type = TypeId::getType<int64_t>();
    static const TypeId uint8Type = TypeId::getType<uint8_t>();
    static const TypeId uint16Type = TypeId::getType<uint16_t>();
    static const TypeId uint32Type = TypeId::getType<uint32_t>();
    static const TypeId uint64Type = TypeId::getType<uint64_t>();
    static const TypeId longType = TypeId::getType<long>();
    static const TypeId ulongType = TypeId::getType<unsigned long>();
    static const TypeId floatType = TypeId::getType<float>();
    static const TypeId doubleType = TypeId::getType<double>();

    if (int8Type == tid)
        return MaxMinRangePair(std::numeric_limits<int8_t>::lowest(), std::numeric_limits<int8_t>::max());

    if (int16Type == tid)
        return MaxMinRangePair(std::numeric_limits<int16_t>::lowest(), std::numeric_limits<int16_t>::max());

    if (int32Type == tid)
        return MaxMinRangePair(std::numeric_limits<int32_t>::lowest(), std::numeric_limits<int32_t>::max());

    if (int64Type == tid)
        return MaxMinRangePair(std::numeric_limits<int64_t>::lowest(), std::numeric_limits<int64_t>::max());

    if (uint8Type == tid)
        return MaxMinRangePair(std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max());

    if (uint16Type == tid)
        return MaxMinRangePair(std::numeric_limits<uint16_t>::min(), std::numeric_limits<uint16_t>::max());

    if (uint32Type == tid)
        return MaxMinRangePair(std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint32_t>::max());

    if (uint64Type == tid)
        return MaxMinRangePair(std::numeric_limits<uint16_t>::min(), std::numeric_limits<uint64_t>::max());

    if (longType == tid)
        return MaxMinRangePair(std::numeric_limits<long>::lowest(), std::numeric_limits<long>::max());

    if (ulongType == tid)
        return MaxMinRangePair(std::numeric_limits<unsigned long>::min(), std::numeric_limits<unsigned long>::max());

    if (floatType == tid)
        return MaxMinRangePair(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());

    if (doubleType == tid)
        return MaxMinRangePair(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());

    return MaxMinRangePair(Variant(), Variant());
    return MaxMinRangePair(Any(), Any());
}*/

//Any getMaxValue(const TypeId & typeId)
//{
//    return getValuePair(typeId).maxValue_;
//}
//
//Variant getMinValue(const TypeId & typeId)
//{
//    return getValuePair(typeId).minValue_;
//}

/*class BatchHolder
{
public:
    BatchHolder(ICommandManager& commandManager_, size_t objectsCount)
        : commandManager(commandManager_)
        , batchStarted(false)
        , batchSuccessed(false)
    {
        if (objectsCount > 1)
        {
            batchStarted = true;
            commandManager.beginBatchCommand();
        }
    }

    ~BatchHolder()
    {
        if (batchStarted == true)
        {
            if (batchSuccessed)
            {
                commandManager.endBatchCommand();
            }
            else
            {
                commandManager.abortBatchCommand();
            }
        }
    }

    void MarkAsSuccessed()
    {
        batchSuccessed = true;
    }

private:
    bool batchStarted;
    bool batchSuccessed;
    ICommandManager& commandManager;
}; */
} // namespace DPMEDetails

void DefaultChildCheatorExtension::ExposeChildren(const std::shared_ptr<const PropertyNode>& node, std::vector<std::shared_ptr<const PropertyNode>>& children) const
{
    DVASSERT(node->reflectedObject.IsValid());

    if (node->propertyType == PropertyNode::RealProperty)
    {
        Vector<Reflection::Field> fields = node->reflectedObject.GetFields();
        for (Reflection::Field& field : fields)
        {
            children.push_back(allocator->CreatePropertyNode(std::move(field.key), std::move(field.ref), PropertyNode::RealProperty));
        }
    }

    return ChildCreatorExtension::ExposeChildren(node, children);
}

class DefaultAllocator : public IChildAllocator
{
public:
    DefaultAllocator();
    ~DefaultAllocator();
    std::shared_ptr<const PropertyNode> CreatePropertyNode(Any&& fieldName, Reflection&& reflection, int32_t type = PropertyNode::RealProperty) override;

private:
    ObjectsPool<PropertyNode, SingleThreadStrategy> pool;
};

DefaultAllocator::DefaultAllocator()
    : pool(10000, 10)
{
}

DefaultAllocator::~DefaultAllocator()
{
}

std::shared_ptr<const PropertyNode> DefaultAllocator::CreatePropertyNode(Any&& fieldName, Reflection&& reflection, int32_t type)
{
    std::shared_ptr<PropertyNode> result = pool.RequestObject();
    result->propertyType = type;
    result->fieldName = std::move(fieldName);
    result->reflectedObject = std::move(reflection);
    result->cachedValue = result->reflectedObject.GetValue();

    return result;
}

std::shared_ptr<IChildAllocator> CreateDefaultAllocator()
{
    return std::make_shared<DefaultAllocator>();
}

ReflectedPropertyItem* DefaultMergeValueExtension::LookUpItem(const std::shared_ptr<const PropertyNode>& node, const std::vector<std::unique_ptr<ReflectedPropertyItem>>& items) const
{
    DVASSERT(node->reflectedObject.IsValid());

    ReflectedPropertyItem* result = nullptr;
    const ReflectedType* valueType = ReflectedType::GetByType(node->reflectedObject.GetValueObject().GetType());

    for (const std::unique_ptr<ReflectedPropertyItem>& item : items)
    {
        const Vector<std::shared_ptr<PropertyNode>>& itemNodes = item->GetPropertyNodes();
        DVASSERT(!itemNodes.empty());
        std::shared_ptr<PropertyNode> etalonNode = itemNodes.front();
        const ReflectedType* etalonItemType = ReflectedType::GetByType(etalonNode->reflectedObject.GetValueObject().GetType());

        if (valueType == etalonItemType && etalonNode->fieldName == node->fieldName)
        {
            result = item.get();
            break;
        }
    }

    return result;
}

} // namespace TArc
} // namespace DAVA
