/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Base/UniquePtr.h"
#include "FileSystem/FileSystem.h"
#include "ResourceArchiver/ResourceArchiver.h"

#include "AssetCache/AssetCacheClient.h"

#include "ArchivePackTool.h"

using namespace DAVA;

namespace OptionNames
{
const DAVA::String Compression = "-compression";
const DAVA::String AddHidden = "-addHidden";
const DAVA::String UseCache = "-useCache";
const DAVA::String Ip = "-ip";
const DAVA::String Port = "-p";
const DAVA::String Timeout = "-t";
const DAVA::String LogFile = "-log";
const DAVA::String Dir = "-dir";
const DAVA::String ListFiles = "-listfile";
const DAVA::String Files = "-file";
};

ArchivePackTool::ArchivePackTool()
    : CommandLineTool("-pack")
{
    options.AddOption(OptionNames::Compression, VariantType(String("lz4hc")), "default compression method, lz4hc - default");
    options.AddOption(OptionNames::AddHidden, VariantType(false), "add hidden files to pack list");
    options.AddOption(OptionNames::UseCache, VariantType(false), "use asset cache");
    options.AddOption(OptionNames::Ip, VariantType(AssetCache::LOCALHOST), "asset cache ip");
    options.AddOption(OptionNames::Port, VariantType(static_cast<uint32>(AssetCache::ASSET_SERVER_PORT)), "asset cache port");
    options.AddOption(OptionNames::Timeout, VariantType(1000u), "asset cache timeout");
    options.AddOption(OptionNames::LogFile, VariantType(String("")), "package process log file");
    options.AddOption(OptionNames::Dir, VariantType(String("")), "source files directory");
    options.AddOption(OptionNames::ListFiles, VariantType(String("")), "text files containing list of source files", true);
    options.AddOption(OptionNames::Files, VariantType(String("")), "source files", true);
    options.AddArgument("packfile");
}

bool ArchivePackTool::ConvertOptionsToParamsInternal()
{
    compressionStr = options.GetOption(OptionNames::Compression).AsString();
    if (!ResourceArchiver::StringToCompressType(compressionStr, compressionType))
    {
        Logger::Error("Invalid compression type: '%s'", compressionStr.c_str());
        return false;
    }

    addHidden = options.GetOption(OptionNames::AddHidden).AsBool();
    useCache = options.GetOption(OptionNames::UseCache).AsBool();
    assetCacheParams.ip = options.GetOption(OptionNames::Ip).AsString();
    assetCacheParams.port = options.GetOption(OptionNames::Port).AsUInt32();
    assetCacheParams.timeoutms = options.GetOption(OptionNames::Timeout).AsUInt32();
    logFileName = options.GetOption(OptionNames::LogFile).AsString();

    source = Source::Unknown;

    srcDir = options.GetOption(OptionNames::Dir).AsString();
    if (!srcDir.empty())
    {
        source = Source::UseDir;
        srcDir = FilePath::MakeDirectory(srcDir);
    }

    uint32 listFilesCount = options.GetOptionValuesCount(OptionNames::ListFiles);
    if (listFilesCount > 1 || options.GetOption(OptionNames::ListFiles, 0).AsString().empty() == false)
    {
        if (source != Source::Unknown)
        {
            Logger::Error("Unexpected parameter: %s", OptionNames::ListFiles.c_str());
            return false;
        }

        source = Source::UseListFiles;

        for (uint32 n = 0; n < listFilesCount; ++n)
        {
            listFiles.push_back(options.GetOption(OptionNames::ListFiles, n).AsString());
        }
    }

    uint32 srcFilesCount = options.GetOptionValuesCount(OptionNames::Files);
    if (srcFilesCount > 1 || (options.GetOption(OptionNames::Files, 0).AsString().empty() == false))
    {
        if (source != Source::Unknown)
        {
            Logger::Error("Unexpected parameter: %s", OptionNames::Files.c_str());
            return false;
        }

        source = Source::UseSrcFiles;

        for (uint32 n = 0; n < srcFilesCount; ++n)
        {
            srcFiles.push_back(options.GetOption(OptionNames::Files, n).AsString());
        }
    }

    packFileName = options.GetArgument("packfile");
    if (packFileName.empty())
    {
        Logger::Error("packfile param is not specified");
        return false;
    }

    return true;
}

void ArchivePackTool::ProcessInternal()
{
    Vector<String> sources;

    FilePath initialDir = FileSystem::Instance()->GetCurrentWorkingDirectory();
    FilePath packFilePath = initialDir + packFileName;
    FilePath logFilePath = (logFileName.empty() ? FilePath() : initialDir + logFileName);

    switch (source)
    {
    case Source::UseDir:
    {
        sources.push_back(srcDir);
        break;
    }
    case Source::UseListFiles:
    {
        for (const String& filename : listFiles)
        {
            UniquePtr<File> listFile(File::Create(filename, File::OPEN | File::READ));
            if (listFile)
            {
                while (!listFile->IsEof())
                {
                    String str = listFile->ReadLine();
                    sources.push_back(str);
                }
            }
            else
            {
                Logger::Error("Can't open listfile %s", filename.c_str());
                return;
            }
        }
        break;
    }
    case Source::UseSrcFiles:
    {
        sources.swap(srcFiles);
    }
    default:
    {
        DVASSERT_MSG(false, Format("Incorrect source type: %d", source));
        return;
    }
    }

    if (sources.empty())
    {
        LOG_ERROR("No source files");
        return;
    }

    if (!logFilePath.IsEmpty())
    {
        FileSystem::Instance()->DeleteFile(logFilePath);
        Logger::Instance()->SetLogPathname(logFilePath);
    }

    std::unique_ptr<AssetCacheClient> assetCache;
    if (useCache)
    {
        assetCache.reset(new AssetCacheClient(true));
        AssetCache::Error result = assetCache->ConnectSynchronously(assetCacheParams);
        if (result != AssetCache::Error::NO_ERRORS)
        {
            LOG_ERROR("Can't connect to asset cache server %s:%u, reason is %s", assetCacheParams.ip, assetCacheParams.port, AssetCache::ErrorToString(result).c_str());
            assetCache.reset();
        }
    }

    ResourceArchiver::CreateArchive(sources, addHidden, compressionType, packFilePath, logFilePath, assetCache.get());

    if (assetCache)
    {
        assetCache->Disconnect();
    }
}
