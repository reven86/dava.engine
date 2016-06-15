#include "CommandLine/Dump/DumpTool.h"
#include "CommandLine/Dump/SceneDumper.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/FileSystem.h"
#include "CommandLine/OptionName.h"

using namespace DAVA;

DumpTool::DumpTool()
    : CommandLineTool("-dump")
{
    options.AddOption(OptionName::Links, VariantType(false), "Target for dumping is links");
    options.AddOption(OptionName::InDir, VariantType(String("")), "Path for Project/DataSource/3d/ folder");
    options.AddOption(OptionName::ProcessFile, VariantType(String("")), "Filename from DataSource/3d/ for dumping");
    options.AddOption(OptionName::QualityConfig, VariantType(String("")), "Full path for quality.yaml file");
    options.AddOption(OptionName::OutFile, VariantType(String("")), "Full path to file to write result of dumping");
}

void DumpTool::ConvertOptionsToParamsInternal()
{
    inFolder = options.GetOption(OptionName::InDir).AsString();
    filename = options.GetOption(OptionName::ProcessFile).AsString();
    qualityPathname = options.GetOption(OptionName::QualityConfig).AsString();
    outFile = options.GetOption(OptionName::OutFile).AsString();

    if (options.GetOption(OptionName::Links).AsBool())
    {
        commandAction = ACTION_DUMP_LINKS;
    }
}

bool DumpTool::InitializeInternal()
{
    if (inFolder.IsEmpty())
    {
        Logger::Error("Input folder was not selected");
        return false;
    }
    inFolder.MakeDirectoryPathname();

    if (filename.empty())
    {
        Logger::Error("Filename was not selected");
        return false;
    }

    if (outFile.IsEmpty())
    {
        Logger::Error("Out file was not selected");
        return false;
    }

    if (commandAction == ACTION_NONE)
    {
        Logger::Error("Target for dumping was not selected");
        return false;
    }

    return true;
}

void DumpTool::ProcessInternal()
{
    if (commandAction == ACTION_DUMP_LINKS)
    {
        auto links = SceneDumper::DumpLinks(inFolder + filename);

        FileSystem::Instance()->CreateDirectory(outFile.GetDirectory(), true);
        ScopedPtr<File> file(File::Create(outFile, File::WRITE | File::CREATE));
        if (file)
        {
            for (const auto& link : links)
            {
                if (!link.IsEmpty() && link.GetType() != FilePath::PATH_IN_MEMORY)
                {
                    file->WriteLine(link.GetAbsolutePathname());
                }
            }
        }
    }
}

DAVA::FilePath DumpTool::GetQualityConfigPath() const
{
    if (qualityPathname.IsEmpty())
    {
        return CreateQualityConfigPath(inFolder);
    }

    return qualityPathname;
}
