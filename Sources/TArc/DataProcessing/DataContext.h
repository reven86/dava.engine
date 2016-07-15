#pragma once

#include "Base/BaseTypes.h"
#include "Base/Type.h"

#include "DataNode.h"

namespace tarc
{

class DataContextListener
{
public:
    virtual ~DataContextListener(); // unsubscribe itself in context

    virtual void DataChanged(const DAVA::Type* type) = 0;

private:
    friend class DataContext;
    void Init(DataContext* context); // called by DataContext on subscription

    DataContext* context = nullptr;
};

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

    void Subscribe(DataContextListener& listener);
    void Unsubscribe(DataContextListener& listener);

protected:
    void SendNotifications();

private:
    DAVA::UnorderedMap<const DAVA::Type*, std::unique_ptr<DataNode>> dataMap;
    DAVA::UnorderedSet<DataContextListener*> listeners;
};

}