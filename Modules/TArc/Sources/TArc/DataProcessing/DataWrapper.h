#pragma once
#define DAVAENGINE_DATAWRAPPER__H

#include "Reflection/Reflection.h"
#include "DataContext.h"

namespace DAVA
{
namespace TArc
{
template <typename T>
class DataEditor;
template <typename T>
class DataReader;
class ReflectedDataEditor;
class DataListener;
class DataWrapper
{
public:
    using DataAccessor = Function<Reflection(const DataContext*)>;

    DataWrapper() = default;
    DataWrapper(const DataWrapper& other) = default;
    DataWrapper& operator=(const DataWrapper& other) = default;

    DataWrapper(DataWrapper&& other);
    DataWrapper& operator=(DataWrapper&& other);

    bool operator==(const DataWrapper& other) const;

    bool HasData() const;
    // you can call SetListener(nullptr) to remove active listener
    void SetListener(DataListener* listener);
    void SetFieldValue(const Any& fieldKey, const Any& value);
    Any GetFieldValue(const Any& fieldKey) const;

    template <typename T>
    DataEditor<T> CreateEditor();

    template <typename T>
    DataReader<T> CreateReader() const;

    bool IsActive() const;

private:
    friend class DataWrappersProcessor;
    friend class QtReflected;
    friend class DataListener;
    template <typename T>
    friend class DataEditor;
    template <typename T>
    friend class DataReader;
    DataWrapper(const ReflectedType* type);
    DataWrapper(const DataAccessor& accessor);

    void SetContext(DataContext* context);
    void ClearListener(DataListener* listenerForCheck);

    void UpdateCachedValue(int32 id, const Any& value);
    void Sync(bool notifyListener);
    void SyncByFieldKey(const Any& fieldKey, const Any& v);
    void SyncWithEditor(const Reflection& etalonData);
    void NotifyListener(bool sendNotify, const Vector<Any>& fields = Vector<Any>());
    Reflection GetData() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

template <typename T>
class DataEditor final
{
public:
    DataEditor(DataWrapper& holder, Reflection reflection);
    ~DataEditor();

    DataEditor(const DataEditor& other) = delete;
    DataEditor& operator=(const DataEditor& other) = delete;

    DataEditor(DataEditor&& other);
    DataEditor& operator=(DataEditor&& other);

    T* operator->();

private:
    Reflection reflection;
    T* dataPtr = nullptr;
    T copyValue;
    DataWrapper holder;
};

template <typename T>
class DataReader final
{
public:
    DataReader(const DataWrapper& holder);

    DataReader(const DataReader& other) = delete;
    DataReader& operator=(const DataReader& other) = delete;

    DataReader(DataReader&& other);
    DataReader& operator=(DataReader&& other);

    T const* operator->() const;

private:
    DataWrapper holder;
};
} // namespace TArc
} // namespace DAVA

#include "TArc/DataProcessing/Private/DataWrapper_impl.h"
