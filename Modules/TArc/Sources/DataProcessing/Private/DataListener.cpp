#include "DataProcessing/DataListener.h"

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
} // namespace TArc
} // namespace DAVA