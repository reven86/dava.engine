#pragma once
#define DAVAENGINE_DATAWRAPPER__H

#include "Reflection/Reflection.h"
#include "DataContext.h"

namespace tarc
{

class DataWrapper : public DAVA::BaseObject
{
public:
    class Listener
    {
    public:
        virtual ~Listener();
        virtual void OnDataChanged(const DataWrapper& wrapper) = 0;

    private:
        friend class DataWrapper;
        void InitListener(DataWrapper& wrapper);

    private:
        DAVA::RefPtr<DataWrapper> holder;
    };

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

        T& operator->();

    private:
        DAVA::Reflection reflection;
        T dataCopy;
        DAVA::RefPtr<DataWrapper> holder;
    };

    using DataAccessor = DAVA::Function<DAVA::Reflection(DataContext&)>;

    template<typename T>
    DataWrapper(DataContext& context, bool listenRecursive = false);
    DataWrapper(std::unique_ptr<DataNode>&& data, DataContext& context, bool listenRecursive = false);
    DataWrapper(const DataAccessor& accessor, DataContext& context, bool listenRecursive = false);

    DataWrapper(const DataWrapper& other) = delete;
    DataWrapper& operator=(const DataWrapper& other) = delete;

    DataWrapper(DataWrapper&& other);
    DataWrapper& operator=(DataWrapper&& other);

    bool HasData() const;
    void AddListener(Listener* listener);
    void RemoveListener(Listener* listener);

    template<typename T>
    Editor<T> CreateEditor();

private:
    void Sync();
    void SyncImpl(const DAVA::Reflection& reflection, DAVA::Vector<DAVA::Any>& values);
    void NotifyListeners();

    static DAVA::Reflection GetDataDefault(DataContext& context, const DAVA::Type* type);

private:
    DataContext& context;
    bool listenRecursive;
    DataAccessor dataAccessor;
    DAVA::Vector<DAVA::Any> cachedValues;

    DAVA::Set<Listener*> listeners;
};

}

#include "Private/DataWrapper_impl.h"