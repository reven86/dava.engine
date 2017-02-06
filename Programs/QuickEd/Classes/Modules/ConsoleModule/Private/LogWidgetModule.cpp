#include "Modules/ConsoleModule/LogWidgetModule.h"
#include "Modules/SpritesPacker/SpritesPackerData.h"
#include "Application/QEGlobal.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

#include <QtTools/ConsoleWidget/LoggerOutputObject.h>
#include <QtTools/ConsoleWidget/LogWidget.h>
#include <QtTools/ReloadSprites/SpritesPacker.h>

#include <Preferences/PreferencesStorage.h>

REGISTER_PREFERENCES_ON_START(LogWidgetModule,
                              PREF_ARG("consoleState", DAVA::String())
                              )

DAVA_VIRTUAL_REFLECTION_IMPL(LogWidgetModule)
{
    DAVA::ReflectionRegistrator<LogWidgetModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void LogWidgetModule::PostInit()
{
    loggerOutput = new LoggerOutputObject();
    connections.AddConnection(loggerOutput, &LoggerOutputObject::OutputReady, DAVA::MakeFunction(this, &LogWidgetModule::OnLogOutput));

    logWidget = new LogWidget();
    DAVA::TArc::DockPanelInfo panelInfo;
    panelInfo.title = "LogWidget";
    panelInfo.area = Qt::BottomDockWidgetArea;
    DAVA::TArc::PanelKey panelKey(QStringLiteral("LogWidget"), panelInfo);
    GetUI()->AddView(QEGlobal::windowKey, panelKey, logWidget);

    PreferencesStorage::Instance()->RegisterPreferences(this);
}

void LogWidgetModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
    PreferencesStorage::Instance()->UnregisterPreferences(this);
    connections.RemoveConnection(loggerOutput, &LoggerOutputObject::OutputReady);
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

DAVA::String LogWidgetModule::GetConsoleState() const
{
    QByteArray consoleState = logWidget->Serialize().toBase64();
    return consoleState.toStdString();
}

void LogWidgetModule::SetConsoleState(const DAVA::String& array)
{
    QByteArray consoleState = QByteArray::fromStdString(array);
    logWidget->Deserialize(QByteArray::fromBase64(consoleState));
}

DECL_GUI_MODULE(LogWidgetModule);
