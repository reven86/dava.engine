#include "CommandLine/DumpTool.h"
#include "CommandLine/Private/Dump/SceneDumper.h"
#include "CommandLine/Private/OptionName.h"
#include "CommandLine/Private/SceneConsoleHelper.h"

#include "FileSystem/FilePath.h"
#include "FileSystem/FileSystem.h"

DumpTool::DumpTool(const DAVA::Vector<DAVA::String>& commandLine)
    : REConsoleModuleCommon(commandLine, "-dump")
{
    using namespace DAVA;

    options.AddOption(OptionName::Links, VariantType(false), "Target for dumping is links");
    options.AddOption(OptionName::InDir, VariantType(String("")), "Path for Project/DataSource/3d/ folder");
    options.AddOption(OptionName::ProcessFile, VariantType(String("")), "Filename from DataSource/3d/ for dumping");
    options.AddOption(OptionName::QualityConfig, VariantType(String("")), "Full path for quality.yaml file");
    options.AddOption(OptionName::OutFile, VariantType(String("")), "Full path to file to write result of dumping");
}

bool DumpTool::PostInitInternal()
{
    inFolder = options.GetOption(OptionName::InDir).AsString();
    if (inFolder.IsEmpty())
    {
        DAVA::Logger::Error("Input folder was not selected");
        return false;
    }
    inFolder.MakeDirectoryPathname();

    filename = options.GetOption(OptionName::ProcessFile).AsString();
    if (filename.empty())
    {
        DAVA::Logger::Error("Filename was not selected");
        return false;
    }

    outFile = options.GetOption(OptionName::OutFile).AsString();
    if (outFile.IsEmpty())
    {
        DAVA::Logger::Error("Out file was not selected");
        return false;
    }

    bool qualityInitialized = SceneConsoleHelper::InitializeQualitySystem(options, inFolder + filename);
    if (!qualityInitialized)
    {
        DAVA::Logger::Error("Cannot create path to quality.yaml from %s", inFolder.GetAbsolutePathname().c_str());
        return false;
    }

    if (options.GetOption(OptionName::Links).AsBool())
    {
        commandAction = ACTION_DUMP_LINKS;
    }
    else
    {
        DAVA::Logger::Error("Target for dumping was not selected");
        return false;
    }

    return true;
}

DAVA::TArc::ConsoleModule::eFrameResult DumpTool::OnFrameInternal()
{
    if (commandAction == ACTION_DUMP_LINKS)
    {
        auto links = SceneDumper::DumpLinks(inFolder + filename);

        DAVA::FileSystem::Instance()->CreateDirectory(outFile.GetDirectory(), true);
        DAVA::ScopedPtr<DAVA::File> file(DAVA::File::Create(outFile, DAVA::File::WRITE | DAVA::File::CREATE));
        if (file)
        {
            for (const auto& link : links)
            {
                if (!link.IsEmpty() && link.GetType() != DAVA::FilePath::PATH_IN_MEMORY)
                {
                    file->WriteLine(link.GetAbsolutePathname());
                }
            }
        }
    }

    return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
}

void DumpTool::BeforeDestroyedInternal()
{
    SceneConsoleHelper::FlushRHI();
}

void DumpTool::ShowHelpInternal()
{
    REConsoleModuleCommon::ShowHelpInternal();

    DAVA::Logger::Info("Examples:");
    DAVA::Logger::Info("\t-dump -indir /Users/SmokeTest/DataSource/3d/ -processfile Maps/11-grass/test_scene.sc2 -outfile /Users/Test/dump.txt -links");
}
