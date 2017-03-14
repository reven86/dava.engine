#pragma once

#include "DataWrapper.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
namespace TArc
{
class DataWrappersProcessor
{
public:
    DataWrappersProcessor() = default;

    void Shoutdown();

    DataWrapper CreateWrapper(const ReflectedType* type, DataContext* ctx);
    DataWrapper CreateWrapper(const DataWrapper::DataAccessor& accessor, DataContext* ctx);

    void SetContext(DataContext* ctx);
    void Sync();

private:
    Vector<DataWrapper> wrappers;
    Vector<DataWrapper> justCreatedWrappers;
    bool recursiveSyncGuard = false;
};
}
}