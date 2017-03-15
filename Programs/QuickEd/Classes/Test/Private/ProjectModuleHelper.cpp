#include "Test/Private/ProjectModuleHelper.h"
#include "Test/Private/TestHelpers.h"

namespace TestHelpers
{
DAVA_VIRTUAL_REFLECTION_IMPL(ProjectModuleHelper)
{
    DAVA::ReflectionRegistrator<ProjectModuleHelper>::Begin()
    .ConstructorByPointer()
    .End();
}

void ProjectModuleHelper::PostInit()
{
    using namespace DAVA;
    using namespace TArc;

    ContextAccessor* accessor = GetAccessor();
    {
        PropertiesItem item = accessor->CreatePropertiesNode(projectsHistoryKey);
        projectsHistory = item.Get<Vector<String>>(recentItemsKey);
        item.Set(recentItemsKey, Vector<String>());
    }
    {
        PropertiesItem item = accessor->CreatePropertiesNode(projectModulePropertiesKey);
        lastProject = item.Get<String>(lastProjectKey);
        item.Set(lastProjectKey, String());
    }
}

ProjectModuleHelper::~ProjectModuleHelper()
{
    using namespace DAVA::TArc;
    ContextAccessor* accessor = GetAccessor();
    {
        PropertiesItem item = accessor->CreatePropertiesNode(projectsHistoryKey);
        item.Set(recentItemsKey, projectsHistory);
    }
    {
        PropertiesItem item = accessor->CreatePropertiesNode(projectModulePropertiesKey);
        item.Set(lastProjectKey, lastProject);
    }
    TestHelpers::ClearTestFolder();
}
} // namespace TestHelpers
