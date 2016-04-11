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

#include "FileSystem/FileSystem.h"
#include "FileSystem/FileList.h"
#include "Utils/Utils.h"
#include "Utils/MD5.h"
#include "Functional/Function.h"
#include "ResourceArchiver/ResourceArchiver.h"
#include "Base/UniquePtr.h"

#include "ArchivePackTool.h"

using namespace DAVA;

static const String DEFAULT_CACHE_IP = "";
static const uint32 DEFAULT_CACHE_PORT = 44234;
static const uint32 DEFAULT_CACHE_TIMEOUT_MS = 5000;

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
    options.AddOption(OptionNames::Ip, VariantType(DEFAULT_CACHE_IP), "asset cache ip");
    options.AddOption(OptionNames::Port, VariantType(DEFAULT_CACHE_PORT), "asset cache port");
    options.AddOption(OptionNames::Timeout, VariantType(DEFAULT_CACHE_TIMEOUT_MS), "asset cache timeout");
    options.AddOption(OptionNames::LogFile, VariantType(String("")), "package process log file");
    options.AddOption(OptionNames::Dir, VariantType(String("")), "source files directory");
    options.AddOption(OptionNames::ListFiles, VariantType(String("")), "text files containing list of source files", true);
    options.AddOption(OptionNames::Files, VariantType(String("")), "source files", true);
    options.AddArgument("packfile");
}

bool ArchivePackTool::ConvertOptionsToParamsInternal()
{
    compressionStr = options.GetOption(OptionNames::Compression).AsString();
    if (!ResourceArchiver::StringToPackType(compressionStr, compressionType))
    {
        Logger::Error("Invalid compression type: '%s'", compressionStr.c_str());
        return false;
    }

    addHidden = options.GetOption(OptionNames::AddHidden).AsBool();
    useCache = options.GetOption(OptionNames::UseCache).AsBool();
    ip = options.GetOption(OptionNames::Ip).AsString();
    port = options.GetOption(OptionNames::Port).AsUInt32();
    timeout = options.GetOption(OptionNames::Timeout).AsUInt32();
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
        //Logger::Info("Packing '%s' into %s", srcDir.c_str(), packFileName.c_str());
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
                    String str;
                    listFile->ReadString(str);
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

    AssetCache::CacheItemKey key;
    if (useCache)
    {
        ConstructCacheKey(key, sources, compressionStr);

        if (RetrieveFromCache(key, packFilePath, logFilePath))
        {
            return;
        }
    }

    if (!logFilePath.IsEmpty())
    {
        Logger::Instance()->SetLogPathname(logFilePath);
    }

    if (ResourceArchiver::CreateArchive(packFilePath, sources, addHidden))
    {
        Logger::Info("done");

        if (useCache)
        {
            AddToCache(key, packFilePath, logFilePath);
        }
        return;
    }
    else
    {
        Logger::Info("packing failed");
        return;
    }
}

MD5::MD5Digest CalculateSourcesMD5(const Vector<String>& sources, bool addHidden)
{
    MD5 md5;
    md5.Init();
    for (const String& source : sources)
    {
        md5.Update(reinterpret_cast<const uint8*>(source.data()), static_cast<uint32>(source.size()));

        FilePath sourcePath(source);

        if (sourcePath.IsDirectoryPathname())
        {
            MD5::RecursiveDirectoryMD5(sourcePath, md5, true, addHidden);
        }
        else
        {
            MD5::MD5Digest fileDigest;
            MD5::ForFile(source, fileDigest);
            md5.Update(fileDigest.digest.data(), static_cast<uint32>(fileDigest.digest.size()));
        }
    }

    md5.Final();
    return md5.GetDigest();
}

void ArchivePackTool::ConstructCacheKey(AssetCache::CacheItemKey& key, const Vector<String>& sources, const DAVA::String& compression) const
{
    MD5::MD5Digest sourcesMD5 = CalculateSourcesMD5(sources, addHidden);
    MD5::MD5Digest paramsMD5;
    MD5::ForData(reinterpret_cast<const uint8*>(compression.data()), compression.size(), paramsMD5);
    AssetCache::SetKeyPart1(key, sourcesMD5);
    AssetCache::SetKeyPart2(key, paramsMD5);
}

bool ArchivePackTool::RetrieveFromCache(const AssetCache::CacheItemKey& key, const FilePath& pathToPackage, const FilePath& pathToLog) const
{
    // todo: retrieve from cache using Viktor's AssetCache interface
    return false;
}

bool ArchivePackTool::AddToCache(const AssetCache::CacheItemKey& key, const FilePath& pathToPack, const FilePath& pathToLog) const
{
    // todo: add to cache using Viktor's AssetCache interface
    return false;
}
