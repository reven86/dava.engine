#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

class LogWidget;
class LoggerOutputObject;

class LogWidgetModule : public DAVA::TArc::ClientModule
{
    void PostInit() override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key) override;

    void OnLogOutput(DAVA::Logger::eLogLevel ll, const QByteArray& output);

    DAVA::TArc::QtConnections connections;

    LoggerOutputObject* loggerOutput = nullptr; //will be deleted by logger. Isn't it fun?
    LogWidget* logWidget = nullptr;

    DAVA_VIRTUAL_REFLECTION(LogWidgetModule, DAVA::TArc::ClientModule);
};
