#include "TArc/DataProcessing/DataListener.h"

namespace DAVA
{
namespace TArc
{
DataListener::~DataListener()
{
    Vector<DataWrapper> wrappersCopy(wrappers.begin(), wrappers.end());
    wrappers.clear();
    for (DataWrapper& wrapper : wrappersCopy)
    {
        wrapper.ClearListener(this);
    }
}

void DataListener::AddWrapper(DataWrapper wrapper)
{
    wrappers.emplace(wrapper);
}

void DataListener::RemoveWrapper(DataWrapper wrapper)
{
    wrappers.erase(wrapper);
}

} // namespace TArc
} // namespace DAVA