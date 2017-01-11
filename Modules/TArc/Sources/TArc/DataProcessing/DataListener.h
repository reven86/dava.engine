#pragma once

#include "DataWrapper.h"

#include "Base/BaseTypes.h"

namespace DAVA
{
namespace TArc
{
class DataListener
{
public:
    virtual ~DataListener();
    virtual void OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& fields) = 0;

private:
    friend class DataWrapper;
    void AddWrapper(DataWrapper wrapper);
    void RemoveWrapper(DataWrapper wrapper);

private:
    struct DataWrapperLess
    {
        bool operator()(const DataWrapper& w1, const DataWrapper& w2) const
        {
            return w1.impl < w2.impl;
        }
    };

    Set<DataWrapper, DataWrapperLess> wrappers;
};
} // namespace TArc
} // namespace DAVA
