#include "Logger/Logger.h"
#include "ResourceArchiver/ResourceArchiver.h"
#include "FileSystem/File.h"

#include "ArchiveListTool.h"
#include "ResultCodes.h"

using namespace DAVA;

namespace OptionNames
{
const DAVA::String Out = "-out";
}

ArchiveListTool::ArchiveListTool()
    : CommandLineTool("list")
{
    options.AddOption(OptionNames::Out, VariantType(String("")), "if specified, program output will be copied to this file");
    options.AddArgument("packfile");
}

bool ArchiveListTool::ConvertOptionsToParamsInternal()
{
    outFilePath = options.GetOption(OptionNames::Out).AsString();
    packFilePath = options.GetArgument("packfile");
    if (packFilePath.IsEmpty())
    {
        Logger::Error("packfile param is not specified");
        return false;
    }

    return true;
}

int ArchiveListTool::ProcessInternal()
{
    try
    {
        ScopedPtr<File> outFile(nullptr);
        if (!outFilePath.IsEmpty())
        {
            outFile.reset(File::Create(outFilePath, File::CREATE | File::WRITE));
        }

        ResourceArchive archive(packFilePath);

        String out = Format("Dumping contents of archive %s", packFilePath.GetFilename().c_str());
        Logger::Info("%s", out.c_str());
        if (outFile)
        {
            outFile->WriteLine(out);
        }

        for (const ResourceArchive::FileInfo& info : archive.GetFilesInfo())
        {
            String compressionStr = GlobalEnumMap<Compressor::Type>::Instance()->ToString(static_cast<int>(info.compressionType));
            out = Format("%s: orig size %u, compressed size %u, type %s",
                         info.relativeFilePath.c_str(), info.originalSize, info.compressedSize, compressionStr.c_str());

            Logger::Info("%s", out.c_str());
            if (outFile)
            {
                outFile->WriteLine(out);
            }
        }

        return ResourceArchiverResult::OK;
    }
    catch (std::exception ex)
    {
        Logger::Error("Can't open archive %s: %s", packFilePath.GetAbsolutePathname().c_str(), ex.what());
        return ResourceArchiverResult::ERROR_CANT_OPEN_ARCHIVE;
    }
}
