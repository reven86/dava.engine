#include "DeadCodeTrick.h"
#include "TArc/Testing/Private/Selftests/CheckBoxTest.h"
#include "TArc/Testing/Private/Selftests/ClientModuleTest.h"
#include "TArc/Testing/Private/Selftests/ContextHierarchyTest.h"
#include "TArc/Testing/Private/Selftests/DataListenerTest.h"
#include "TArc/Testing/Private/Selftests/DoubleSpinBoxTests.h"
#include "TArc/Testing/Private/Selftests/FieldBinderTest.h"
#include "TArc/Testing/Private/Selftests/IntSpinBoxTests.h"
#include "TArc/Testing/Private/Selftests/LineEditTest.h"
#include "TArc/Testing/Private/Selftests/SceneTabbarTest.h"

namespace DAVA
{
namespace TArc
{
bool AvoidTestsStriping()
{
    return true;
}
} // namespace TArc
} // namespace DAVA