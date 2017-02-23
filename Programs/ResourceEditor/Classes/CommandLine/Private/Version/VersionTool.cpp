#include "CommandLine/VersionTool.h"
#include "Logger/Logger.h"

#include <Tools/Version.h>

#include "DAVAVersion.h"
#include "TArc/Utils/ModuleCollection.h"

#include <QtGlobal>

VersionTool::VersionTool(const DAVA::Vector<DAVA::String>& commandLine)
    : CommandLineModule(commandLine, "-version")
{
}

DAVA::TArc::ConsoleModule::eFrameResult VersionTool::OnFrameInternal()
{
    DAVA::Logger::Info("========================================");
    DAVA::Logger::Info("Qt: %s", QT_VERSION_STR);
    DAVA::Logger::Info("Engine: %s", DAVAENGINE_VERSION);
    DAVA::Logger::Info("Appication: %s", APPLICATION_BUILD_VERSION);
    DAVA::Logger::Info("%u bit", static_cast<DAVA::uint32>(sizeof(DAVA::pointer_size) * 8));
    DAVA::Logger::Info("========================================");

    return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
}

DECL_CONSOLE_MODULE(VersionTool, "-version");
