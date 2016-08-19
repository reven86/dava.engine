#pragma once
#define DAVAENGINE_DATAWRAPPER__H

#include "Reflection/Reflection.h"
#include "DataContext.h"

namespace tarc
{

template<typename T> class DataEditor;
class DataListener;
class DataWrapper
{
public:
    using DataAccessor = DAVA::Function<DAVA::Reflection(const DataContext&)>;

    DataWrapper() = default;
    DataWrapper(const DataWrapper& other) = default;
    DataWrapper& operator=(const DataWrapper& other) = default;

    DataWrapper(DataWrapper&& other);
    DataWrapper& operator=(DataWrapper&& other);

    bool HasData() const;
    void AddListener(DataListener* listener);
    void RemoveListener(DataListener* listener);

    template<typename T>
    DataEditor<T> CreateEditor();

private:
    friend class Core;
    friend class QtReflected;
    template<typename T> friend class DataEditor;
    DataWrapper(const DAVA::ReflectedType* type);
    DataWrapper(const DataAccessor& accessor);

    void SetContext(DataContext* context);

    void Sync(bool notifyListeners);
    void NotifyListeners(bool sendNotify, const DAVA::Set<DAVA::String>& fields = DAVA::Set<DAVA::String>());
    DAVA::Reflection GetData() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};
    
template<typename T>
class DataEditor final
{
public:
    DataEditor(DataWrapper& holder, DAVA::Reflection reflection);
    ~DataEditor();
    
    DataEditor(const DataEditor& other) = delete;
    DataEditor& operator=(const DataEditor& other) = delete;
    
    DataEditor(DataEditor&& other);
    DataEditor& operator=(DataEditor&& other);
    
    T* operator->();
    
private:
    DAVA::Reflection reflection;
    T* dataPtr = nullptr;
    DataWrapper holder;
};

class DataListener
{
public:
    virtual ~DataListener();
    virtual void OnDataChanged(const DataWrapper& wrapper, const DAVA::Set<DAVA::String>& fields) = 0;

private:
    friend class DataWrapper;
    void InitListener(const DataWrapper& wrapper);

private:
    DataWrapper holder;
};

}

#include "DataProcessing/Private/DataWrapper_impl.h"