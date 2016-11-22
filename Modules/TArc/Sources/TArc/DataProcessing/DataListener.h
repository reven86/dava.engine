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
    virtual void OnDataChanged(const DataWrapper& wrapper, const Set<String>& fields) = 0;

private:
    friend class DataWrapper;
    void InitListener(const DataWrapper& wrapper);
    void Clear();

private:
    DataWrapper holder;
};
} // namespace TArc
} // namespace DAVA
