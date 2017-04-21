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

#include <FileSystem/VariantType.h>

#include <QByteArray>
#include <QColor>
#include <QVector>

namespace SettingsModuleDetail
{
void ApplyColorPickerSettins(DAVA::TArc::ContextAccessor* contextAccessor)
{
    DAVA::TArc::PropertiesItem propsItem = contextAccessor->CreatePropertiesNode("ColorPickerDialogProperties");

    { // color multiplier
        DAVA::float32 maxMul = SettingsManager::Instance()->GetValue(Settings::General_ColorMultiplyMax).AsFloat();
        propsItem.Set("CPD_maxMultiplier", maxMul);
    }

    { //palette
        const DAVA::String PALETTE_KEY = "CPD_palette";
        QByteArray savedPaletteData = propsItem.Get<QByteArray>(PALETTE_KEY);
        if (savedPaletteData.size() == 0)
        {
            DAVA::VariantType v = SettingsManager::Instance()->GetValue(Settings::Internal_CustomPalette);
            DAVA::int32 vSize = v.AsByteArraySize();
            DAVA::int32 n = vSize / sizeof(DAVA::int32);
            const DAVA::uint32* a = reinterpret_cast<const DAVA::uint32*>(v.AsByteArray());

            QByteArray paletteData;
            QDataStream paletteStream(&paletteData, QIODevice::WriteOnly);

            for (int i = 0; i < n; i++)
            {
                paletteStream << a[i];
            }

            propsItem.Set(PALETTE_KEY, DAVA::Any(paletteData));
        }
    }
}
}

void SettingsModule::PostInit()
{
    using namespace DAVA::TArc;

    QtAction* settingsAction = new QtAction(GetAccessor(), QIcon(":/QtIcons/settings.png"), QString("Settings"));

    ActionPlacementInfo menuPlacement(CreateMenuPoint("Tools"));
    GetUI()->AddAction(DAVA::TArc::mainWindowKey, menuPlacement, settingsAction);

    connections.AddConnection(settingsAction, &QAction::triggered, DAVA::MakeFunction(this, &SettingsModule::ShowSettings));

    { //apply settings
        SettingsModuleDetail::ApplyColorPickerSettins(GetAccessor());
    }
}

void SettingsModule::ShowSettings()
{
    { //show and edit settings
        SettingsDialog dlg(GetUI()->GetWindow(DAVA::TArc::mainWindowKey));
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
