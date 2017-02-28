#include "Classes/Settings/SettingsModule.h"
#include "Classes/Settings/SettingsManager.h"
#include "Classes/Settings/Settings.h"
#include "Classes/Settings/Private/SettingsDialog.h"

#include "Classes/Application/REGlobal.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/PropertiesHolder.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/Utils/ModuleCollection.h>

namespace SettingsModuleDetail
{
void ApplyColorPickerSettins(DAVA::TArc::ContextAccessor* contextAccessor)
{
    const String PROPERTIES_KEY = "ColorPickerDialogProperties";
    const String MULTIPLIER_KEY = "CPD_maxMultiplier";

    DAVA::TArc::PropertiesItem propsItem = contextAccessor->CreatePropertiesNode(PROPERTIES_KEY);

    float32 maxMul = SettingsManager::Instance()->GetValue(Settings::General_ColorMultiplyMax).AsFloat();
    propsItem.Set(MULTIPLIER_KEY, maxMul);
}
}

void SettingsModule::PostInit()
{
    using namespace DAVA::TArc;

    QtAction* settingsAction = new QtAction(GetAccessor(), QIcon(":/QtIcons/settings.png"), QString("Settings"));

    ActionPlacementInfo menuPlacement(CreateMenuPoint("Tools"));
    GetUI()->AddAction(REGlobal::MainWindowKey, menuPlacement, settingsAction);

    connections.AddConnection(settingsAction, &QAction::triggered, DAVA::MakeFunction(this, &SettingsModule::ShowSettings));
}

void SettingsModule::ShowSettings()
{
    { //show and edit settings
        SettingsDialog dlg(GetUI()->GetWindow(REGlobal::MainWindowKey));
        dlg.exec();
    }

    { //apply settings
        SettingsModuleDetail::ApplyColorPickerSettins(GetAccessor());
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(SettingsModule)
{
    DAVA::ReflectionRegistrator<SettingsModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_GUI_MODULE(SettingsModule);
