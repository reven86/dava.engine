#pragma once

#include "Base/BaseTypes.h"
#include "Base/Type.h"

#include "Functional/Function.h"

#include "DataNode.h"

namespace tarc
{

class DataContext
{
public:
    DataContext();
    ~DataContext();

    void CreateData(std::unique_ptr<DataNode>&& node);

    template<typename T>
    bool HasData() const;

    template<typename T>
    T& GetData() const; // throw std::runtime_exception if T not exists

    template<typename T>
    void DeleteData();

    bool HasData(const DAVA::Type* type);
    DataNode& GetData(const DAVA::Type* type); // throw std::runtime_exception if T not exists
    void DeleteData(const DAVA::Type* type);

    void RegisterAction(int id, const DAVA::Function<void(const DAVA::Any& args)>& action);
    void UnregisterAction(int id);
    void CallAction(int id, const DAVA::Any& args);

private:
    //DAVA::UnorderedMap<const DAVA::Type*, std::unique_ptr<DataNode>> dataMap;
};

}