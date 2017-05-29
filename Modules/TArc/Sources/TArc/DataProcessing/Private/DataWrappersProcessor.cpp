#include "TArc/DataProcessing/DataWrappersProcessor.h"

#include "Utils/Utils.h"

namespace DAVA
{
namespace TArc
{
void DataWrappersProcessor::Shoutdown()
{
    wrappers.clear();
    justCreatedWrappers.clear();
}

void DataWrappersProcessor::SetDebugName(const String& debugName_)
{
    debugName = debugName_;
}

DataWrapper DataWrappersProcessor::CreateWrapper(const ReflectedType* type, DataContext* ctx)
{
    DataWrapper wrapper(type);
    wrapper.SetContext(ctx);
    justCreatedWrappers.push_back(wrapper);
    return wrapper;
}

DataWrapper DataWrappersProcessor::CreateWrapper(const DataWrapper::DataAccessor& accessor, DataContext* ctx)
{
    DataWrapper wrapper(accessor);
    wrapper.SetContext(ctx);
    justCreatedWrappers.push_back(wrapper);
    wrapper.SetDebugName(debugName);
    return wrapper;
}

void DataWrappersProcessor::SetContext(DataContext* ctx)
{
    for (DataWrapper& wrapper : wrappers)
    {
        wrapper.SetContext(ctx);
    }

    for (DataWrapper& wrapper : justCreatedWrappers)
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
    wrappers.insert(wrappers.end(), justCreatedWrappers.begin(), justCreatedWrappers.end());
    justCreatedWrappers.clear();

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
