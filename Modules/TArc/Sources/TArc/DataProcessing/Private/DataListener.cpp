#include "TArc/DataProcessing/DataListener.h"

namespace DAVA
{
namespace TArc
{
DataListener::~DataListener()
{
    holder.RemoveListener(this);
}

void DataListener::InitListener(const DataWrapper& wrapper)
{
    holder = wrapper;
}

void DataListener::Clear()
{
    holder = DataWrapper();
}

} // namespace TArc
} // namespace DAVA