#include "ArchivePackTool.h"

#include <FileSystem/FileSystem.h>
#include <Tools/ResourceArchiver/ResourceArchiver.h>
#include <Tools/AssetCache/AssetCacheClient.h>
#include <Utils/StringUtils.h>
#include <Utils/StringFormat.h>
#include <Logger/Logger.h>
#include <Debug/DVAssert.h>
#include <Engine/Engine.h>
#include "ResultCodes.h"

namespace OptionNames
{
const DAVA::String Compression = "-compression";
const DAVA::String AddHidden = "-addHidden";
const DAVA::String UseCache = "-useCache";
const DAVA::String Ip = "-ip";
const DAVA::String Port = "-p";
const DAVA::String Timeout = "-t";
const DAVA::String LogFile = "-log";
const DAVA::String Src = "-src";
const DAVA::String ListFile = "-listfile";
const DAVA::String BaseDir = "-basedir";
const DAVA::String MetaDbFile = "-metadb";
const DAVA::String GenDvpl = "-dvpl";
};

ArchivePackTool::ArchivePackTool()
    : CommandLineTool("pack")
{
    using namespace DAVA;
    static const uint32 defaultPort = static_cast<uint32>(AssetCache::ASSET_SERVER_PORT);
    static const uint64 defaultTimeout = 1000ul;
    options.AddOption(OptionNames::Compression, VariantType(String("lz4hc")), "default compression method, lz4hc - default");
    options.AddOption(OptionNames::AddHidden, VariantType(false), "add hidden files to pack list");
    options.AddOption(OptionNames::UseCache, VariantType(false), "use asset cache");
    options.AddOption(OptionNames::Ip, VariantType(AssetCache::GetLocalHost()), "asset cache ip");
    options.AddOption(OptionNames::Port, VariantType(defaultPort), "asset cache port");
    options.AddOption(OptionNames::Timeout, VariantType(defaultTimeout), "asset cache timeout");
    options.AddOption(OptionNames::LogFile, VariantType(String("")), "package process log file");
    options.AddOption(OptionNames::Src, VariantType(String("")), "source files directory", true);
    options.AddOption(OptionNames::ListFile, VariantType(String("")), "text files containing list of source files", true);
    options.AddOption(OptionNames::BaseDir, VariantType(String("")), "source base directory");
    options.AddOption(OptionNames::MetaDbFile, VariantType(String("")), "sqlite db with metadata");
    options.AddOption(OptionNames::GenDvpl, VariantType(false), "generate dvpl's into ouput directory");
    options.AddArgument("packfile");
}

bool ArchivePackTool::ConvertOptionsToParamsInternal()
{
    using namespace DAVA;
    compressionStr = options.GetOption(OptionNames::Compression).AsString();

    int type;
    if (!GlobalEnumMap<Compressor::Type>::Instance()->ToValue(compressionStr.c_str(), type))
    {
        Logger::Error("Invalid compression type: '%s'", compressionStr.c_str());
        return false;
    }
    compressionType = static_cast<Compressor::Type>(type);

    addHidden = options.GetOption(OptionNames::AddHidden).AsBool();
    useCache = options.GetOption(OptionNames::UseCache).AsBool();
    assetCacheParams.ip = options.GetOption(OptionNames::Ip).AsString();
    assetCacheParams.port = static_cast<uint16>(options.GetOption(OptionNames::Port).AsUInt32());
    assetCacheParams.timeoutms = options.GetOption(OptionNames::Timeout).AsUInt64();
    logFileName = options.GetOption(OptionNames::LogFile).AsString();
    baseDir = options.GetOption(OptionNames::BaseDir).AsString();
    metaDbPath = options.GetOption(OptionNames::MetaDbFile).AsString();

    source = Source::Unknown;

    uint32 listFilesCount = options.GetOptionValuesCount(OptionNames::ListFile);
    if (listFilesCount > 1 || options.GetOption(OptionNames::ListFile, 0).AsString().empty() == false)
    {
        if (source != Source::Unknown)
        {
            Logger::Error("Unexpected parameter: %s", OptionNames::ListFile.c_str());
            return false;
        }

        source = Source::UseListFiles;

        for (uint32 n = 0; n < listFilesCount; ++n)
        {
            listFiles.push_back(options.GetOption(OptionNames::ListFile, n).AsString());
        }
    }

    uint32 srcFilesCount = options.GetOptionValuesCount(OptionNames::Src);
    if (srcFilesCount > 1 || (options.GetOption(OptionNames::Src, 0).AsString().empty() == false))
    {
        if (source != Source::Unknown)
        {
            Logger::Error("Unexpected parameter: %s", OptionNames::Src.c_str());
            return false;
        }

        source = Source::UseSrc;

        for (uint32 n = 0; n < srcFilesCount; ++n)
        {
            srcFiles.push_back(options.GetOption(OptionNames::Src, n).AsString());
        }
    }

    if (!metaDbPath.empty())
    {
        source = Source::UseMetaDB;
    }

    if (source == Source::Unknown)
    {
        Logger::Error("Source is not specified. Either -src or -listfile or -metadb should be added");
        return false;
    }

    packFileName = options.GetArgument("packfile");
    if (packFileName.empty())
    {
        Logger::Error("packfile param is not specified");
        return false;
    }

    genDvpl = options.GetArgument(OptionNames::GenDvpl) != "";

    if (genDvpl)
    {
        FilePath outputName = packFileName;
        if (!outputName.IsDirectoryPathname() || !FileSystem::Instance()->IsDirectory(outputName))
        {
            Logger::Error("packfile should be directory name if using %s", OptionNames::GenDvpl.c_str());
            return false;
        }
    }

    return true;
}

int ArchivePackTool::ProcessInternal()
{
    using namespace DAVA;
    Vector<String> sources;

    switch (source)
    {
    case Source::UseListFiles:
    {
        for (const String& filename : listFiles)
        {
            ScopedPtr<File> listFile(File::Create(filename, File::OPEN | File::READ));
            if (listFile)
            {
                while (!listFile->IsEof())
                {
                    String str = StringUtils::Trim(listFile->ReadLine());
                    if (!str.empty())
                    {
                        sources.push_back(str);
                    }
                }
            }
            else
            {
                Logger::Error("Can't open listfile %s", filename.c_str());
                return ResourceArchiverResult::ERROR_CANT_OPEN_FILE;
            }
        }
        break;
    }
    case Source::UseSrc:
    {
        sources.swap(srcFiles);
        break;
    }
    case Source::UseMetaDB:
        // do nothing
        break;
    default:
    {
        DVASSERT(false, DAVA::Format("Incorrect source type: %d", source).c_str());
        return ResourceArchiverResult::ERROR_INTERNAL;
    }
    }

    FilePath logFilePath(logFileName);
    if (!logFilePath.IsEmpty())
    {
        FileSystem::Instance()->DeleteFile(logFilePath);
        GetEngineContext()->logger->SetLogPathname(logFilePath);
    }

    std::unique_ptr<AssetCacheClient> assetCache;
    if (useCache)
    {
        assetCache.reset(new AssetCacheClient());
        AssetCache::Error result = assetCache->ConnectSynchronously(assetCacheParams);
        if (result != AssetCache::Error::NO_ERRORS)
        {
            Logger::Error("Can't connect to asset cache server %s:%u, reason is %s", assetCacheParams.ip.c_str(), assetCacheParams.port, AssetCache::ErrorToString(result).c_str());
            assetCache.reset();
        }
    }

    ResourceArchiver::Params params;
    params.sourcesList.swap(sources);
    params.addHiddenFiles = addHidden;
    params.compressionType = compressionType;
    params.archivePath = packFileName;
    params.logPath = logFilePath;
    params.assetCacheClient = assetCache.get();
    params.baseDirPath = (baseDir.empty() ? FileSystem::Instance()->GetCurrentWorkingDirectory() : baseDir);
    params.metaDbPath = metaDbPath;

    CreateArchive(params);

    if (assetCache)
    {
        assetCache->Disconnect();
    }

    return ResourceArchiverResult::OK;
}
