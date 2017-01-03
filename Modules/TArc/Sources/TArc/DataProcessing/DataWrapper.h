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

    template <typename T>
    DataEditor<T> CreateEditor();

    bool IsActive() const;

private:
    friend class DataWrappersProcessor;
    friend class QtReflected;
    friend class DataListener;
    template <typename T>
    friend class DataEditor;
    DataWrapper(const ReflectedType* type);
    DataWrapper(const DataAccessor& accessor);

    void SetContext(DataContext* context);
    void ClearListener(DataListener* listenerForCheck);

    void UpdateCachedValue(int32 id, const Any& value);
    void Sync(bool notifyListener);
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
} // namespace TArc
} // namespace DAVA

#include "TArc/DataProcessing/Private/DataWrapper_impl.h"