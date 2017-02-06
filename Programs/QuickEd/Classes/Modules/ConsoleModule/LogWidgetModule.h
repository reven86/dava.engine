#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <Base/Introspection.h>

class LogWidget;
class LoggerOutputObject;

class LogWidgetModule : public DAVA::TArc::ClientModule, public DAVA::InspBase
{
    void PostInit() override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key) override;

    void OnLogOutput(DAVA::Logger::eLogLevel ll, const QByteArray& output);

    DAVA::String GetConsoleState() const;
    void SetConsoleState(const DAVA::String& array);

    DAVA::TArc::QtConnections connections;

    LoggerOutputObject* loggerOutput = nullptr; //will be deleted by logger. Isn't it fun?
    LogWidget* logWidget = nullptr;

    DAVA_VIRTUAL_REFLECTION(LogWidgetModule, DAVA::TArc::ClientModule);

public:
    INTROSPECTION(LogWidgetModule,
                  PROPERTY("consoleState", "LogWidgetModuleInternal/ConsoleState", GetConsoleState, SetConsoleState, DAVA::I_PREFERENCE)
                  )
};
