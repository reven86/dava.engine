#include "TArc/DataProcessing/DataWrappersProcessor.h"

#include "Utils/Utils.h"

namespace DAVA
{
namespace TArc
{
void DataWrappersProcessor::Shoutdown()
{
    wrappers.clear();
}

DataWrapper DataWrappersProcessor::CreateWrapper(const ReflectedType* type, DataContext* ctx)
{
    DataWrapper wrapper(type);
    wrapper.SetContext(ctx);
    wrappers.push_back(wrapper);
    return wrapper;
}

DataWrapper DataWrappersProcessor::CreateWrapper(const DataWrapper::DataAccessor& accessor, DataContext* ctx)
{
    DataWrapper wrapper(accessor);
    wrapper.SetContext(ctx);
    wrappers.push_back(wrapper);
    return wrapper;
}

void DataWrappersProcessor::SetContext(DataContext* ctx)
{
    for (DataWrapper& wrapper : wrappers)
    {
        wrapper.SetContext(ctx);
    }
}

void DataWrappersProcessor::Sync()
{
    if (recursiveSyncGuard == true)
    {
        return;
    }
    recursiveSyncGuard = true;
    size_t index = 0;
    while (index < wrappers.size())
    {
        if (!wrappers[index].IsActive())
        {
            DAVA::RemoveExchangingWithLast(wrappers, index);
        }
        else
        {
            ++index;
        }
    }
    for (DataWrapper& wrapper : wrappers)
    {
        wrapper.Sync(true);
    }
    recursiveSyncGuard = false;
}
}
}
