#include "Modules/PreferencesModule/PreferencesModule.h"
#include "Modules/PreferencesModule/PreferencesData.h"

#include <TArc/Utils/ModuleCollection.h>

DAVA_VIRTUAL_REFLECTION_IMPL(PreferencesModule)
{
    DAVA::ReflectionRegistrator<PreferencesModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void PreferencesModule::PostInit()
{
    using namespace DAVA::TArc;

    std::unique_ptr<PreferencesData> preferencesData(new PreferencesData());

    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    globalContext->CreateData(std::move(preferencesData));
}

DECL_GUI_MODULE(PreferencesModule);
