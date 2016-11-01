#include "Classes/Qt/Application/REGlobal.h"

#include "TArc/Core/Private/CoreInterface.h"

namespace REGlobal
{
namespace REGlobalDetails
{
DAVA::TArc::CoreInterface* coreInstance = nullptr;
}

DAVA::TArc::DataContext& GetGlobalContext()
{
    DVASSERT(REGlobalDetails::coreInstance != nullptr);
    return REGlobalDetails::coreInstance->GetGlobalContext();
}

void InitTArcCore(DAVA::TArc::CoreInterface* core)
{
    REGlobalDetails::coreInstance = core;
}

} // namespace REGlobal