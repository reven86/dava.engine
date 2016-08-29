#pragma once
#define DAVAENGINE_DATACONTEXT__H

#include "Base/BaseTypes.h"
#include "Base/Type.h"

#include "Functional/Function.h"

#include "DataNode.h"

namespace DAVA
{
namespace TArc
{
class DataContext
{
public:
    DataContext() = default;
    ~DataContext();

    void CreateData(std::unique_ptr<DataNode>&& node);

    template<typename T>
    bool HasData() const;

    template<typename T>
    T& GetData() const; // throw std::runtime_exception if T not exists

    template<typename T>
    void DeleteData();

    bool HasData(const DAVA::ReflectedType* type) const;
    DataNode& GetData(const DAVA::ReflectedType* type) const; // throw std::runtime_exception if T not exists
    void DeleteData(const DAVA::ReflectedType* type);

    /*void RegisterAction(int id, const DAVA::Function<void(const DAVA::Any& args)>& action);
    void UnregisterAction(int id);
    void CallAction(int id, const DAVA::Any& args);*/

    using ContextID = DAVA::uint64;
    ContextID GetID() const;

    static const ContextID Empty = 0;

private:
    DAVA::UnorderedMap<const DAVA::ReflectedType*, DataNode*> dataMap;
};
} // namespace TArc
} // namespace DAVA

#include "DataProcessing/Private/DataContext_impl.h"