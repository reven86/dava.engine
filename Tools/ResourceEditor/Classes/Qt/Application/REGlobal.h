#pragma once

#include "Base/FastName.h"

namespace DAVA
{
namespace TArc
{
class CoreInterface;
class DataContext;
}
}

namespace REGlobal
{
DAVA::FastName MainWindowName = DAVA::FastName("ResourceEditor");

DAVA::TArc::DataContext& GetGlobalContext();
void InitTArcCore(DAVA::TArc::CoreInterface* core);
}