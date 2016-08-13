#pragma once
#define DAVAENGINE_DATAWRAPPER__H

#include "Reflection/Reflection.h"
#include "DataContext.h"

namespace tarc
{

class DataListener;
class DataWrapper
{
public:
    template<typename T>
    class Editor
    {
    public:
        Editor(DataWrapper& holder, DAVA::Reflection reflection);
        ~Editor();

        Editor(const Editor& other) = delete;
        Editor& operator=(const Editor& other) = delete;

        Editor(Editor&& other);
        Editor& operator=(Editor&& other);

        T* operator->();

    private:
        DAVA::Reflection reflection;
        T* dataCopy;
        DataWrapper holder;
    };

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
    Editor<T> CreateEditor();

private:
    friend class Core;
    friend class QtReflected;
    DataWrapper(const DAVA::Type* type);
    DataWrapper(const DataAccessor& accessor);

    void SetContext(DataContext* context);

    void Sync(bool notifyListeners);
    void NotifyListeners(bool sendNotify, const DAVA::Set<DAVA::String>& fields = DAVA::Set<DAVA::String>());
    DAVA::Reflection GetData() const;

    static DAVA::Reflection GetDataDefault(const DataContext& context, const DAVA::Type* type);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
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