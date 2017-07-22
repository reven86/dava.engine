#include "Modules/ConsoleModule/LogWidgetModule.h"
#include "Modules/SpritesPacker/SpritesPackerData.h"
#include "Application/QEGlobal.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

#include <QtTools/ConsoleWidget/LoggerOutputObject.h>
#include <QtTools/ConsoleWidget/LogWidget.h>
#include <QtTools/ReloadSprites/SpritesPacker.h>

DAVA_VIRTUAL_REFLECTION_IMPL(LogWidgetModule)
{
    DAVA::ReflectionRegistrator<LogWidgetModule>::Begin()
    .ConstructorByPointer()
    .End();
}

namespace LogWidgetModuleDetails
{
const char* logWidgetModulePropertiesName = "LogWidgetModule Properties";
const char* logWidgetPropertiesName = "LogWidget properties";
}

void LogWidgetModule::PostInit()
{
    loggerOutput = new LoggerOutputObject();
    connections.AddConnection(loggerOutput, &LoggerOutputObject::OutputReady, DAVA::MakeFunction(this, &LogWidgetModule::OnLogOutput));

    logWidget = new LogWidget();

    DAVA::TArc::PropertiesItem item = GetAccessor()->CreatePropertiesNode(LogWidgetModuleDetails::logWidgetModulePropertiesName);
    logWidget->Deserialize(item.Get<QByteArray>(LogWidgetModuleDetails::logWidgetPropertiesName));

    const char* title = "Console";
    DAVA::TArc::DockPanelInfo panelInfo;
    panelInfo.title = title;
    panelInfo.area = Qt::BottomDockWidgetArea;
    DAVA::TArc::PanelKey panelKey(title, panelInfo);
    GetUI()->AddView(DAVA::TArc::mainWindowKey, panelKey, logWidget);
}

void LogWidgetModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
    connections.RemoveConnection(loggerOutput, &LoggerOutputObject::OutputReady);

    DAVA::TArc::PropertiesItem item = GetAccessor()->CreatePropertiesNode(LogWidgetModuleDetails::logWidgetModulePropertiesName);
    item.Set(LogWidgetModuleDetails::logWidgetPropertiesName, logWidget->Serialize());
}

void LogWidgetModule::OnLogOutput(DAVA::Logger::eLogLevel ll, const QByteArray& output)
{
    if (ll != DAVA::Logger::LEVEL_ERROR && ll != DAVA::Logger::LEVEL_WARNING)
    {
        const SpritesPackerData* spritesPackerData = GetAccessor()->GetGlobalContext()->GetData<SpritesPackerData>();
        if (spritesPackerData != nullptr)
        {
            const SpritesPacker* spritesPacker = spritesPackerData->GetSpritesPacker();
            if (spritesPacker->IsRunning())
            {
                return;
            }
        }
    }
    logWidget->AddMessage(ll, output);
}

DECL_GUI_MODULE(LogWidgetModule);
