#include "CommandLine/Version/VersionTool.h"
#include "Logger/Logger.h"

#include "DAVAVersion.h"
#include "Version.h"


#include <QtGlobal>
#include <QString>

using namespace DAVA;

VersionTool::VersionTool()
    : CommandLineTool("-version")
{
}

void VersionTool::ConvertOptionsToParamsInternal()
{
}

bool VersionTool::InitializeInternal()
{
    return true;
}

void VersionTool::ProcessInternal()
{
    auto logLevel = DAVA::Logger::Instance()->GetLogLevel();
    DAVA::Logger::Instance()->SetLogLevel(DAVA::Logger::LEVEL_INFO);

    DAVA::Logger::Info("========================================");
    DAVA::Logger::Info("Qt: %s", QT_VERSION_STR);
    DAVA::Logger::Info("Engine: %s", DAVAENGINE_VERSION);
    DAVA::Logger::Info("Appication: %s", APPLICATION_BUILD_VERSION);
    DAVA::Logger::Info("%u bit", static_cast<DAVA::uint32>(sizeof(DAVA::pointer_size) * 8));
    DAVA::Logger::Info("========================================");

    DAVA::Logger::Instance()->SetLogLevel(logLevel);
}
