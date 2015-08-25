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
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include <iostream>

#include <FileSystem/FileSystem.h>
#include <FileSystem/ResourceArchive.h>
#include <FileSystem/FileList.h>
#include <CommandLine/ProgramOptions.h>

using namespace DAVA;

template <typename T>
using Deleter = void (*)(T* obj);

void FrameworkDidLaunched()
{
}

void FrameworkWillTerminate()
{
}

void CollectAllFilesInDirectory(const String& pathDirName,
                                Vector<String>& fileNameList)
{
    FilePath pathToDir = pathDirName;
    auto includeHidden = false;
    auto* fileList = new FileList(pathToDir, includeHidden);
    for (auto file = 0; file < fileList->GetCount(); ++file)
    {
        if (fileList->IsDirectory(file))
        {
            auto directoryName = fileList->GetFilename(file);
            if ((directoryName != "..") && (directoryName != "."))
            {
                CollectAllFilesInDirectory(directoryName + '/', fileNameList);
            }
        }
        else
        {
            auto filename = fileList->GetFilename(file);
            auto pathname = pathDirName + filename;
            fileNameList.push_back(pathname);
        }
    }
    fileList->Release();
}

int PackDirectoryIntoPakfile(const String& dir,
                             const String& pakfileName,
                             const ResourceArchive::Rules& compressionRules)
{
    auto result = EXIT_FAILURE;

    std::cout << "===================================================\n"
              << "=== Packer started\n"
              << "=== Pack directory: " << dir << '\n'
              << "=== Pack archiveName: " << pakfileName << '\n'
              << "===================================================\n";

    auto dirWithSlash = (dir.back() == '/' ? dir : dir + '/');

    std::unique_ptr<FileSystem, Deleter<FileSystem>> fileSystem(
        new FileSystem(), [](FileSystem* fs)
        {
            SafeDelete(fs);
        });

    fileSystem->SetCurrentWorkingDirectory(dir);

    Vector<String> filesList;

    CollectAllFilesInDirectory("", filesList);

    if (filesList.empty())
    {
        Logger::Error("no files found in: %s\n", dir.c_str());
        return EXIT_FAILURE;
    }

    std::sort(begin(filesList), end(filesList),
              [](const String& one, const String& two)
              {
                  return one < two;
              });

    ResourceArchive::CreatePack(pakfileName, filesList, compressionRules);

    return result;
}

int UnpackPackfileIntoDirectory(const String& pak, const String& dir)
{
    auto fs = new FileSystem();
    DVASSERT(fs);
    auto programmPath = fs->GetCurrentWorkingDirectory().GetAbsolutePathname();

    auto pathArchiveNameDir = programmPath + "/" + pak;

    std::cout << "===================================================\n"
              << "=== Unpacker started\n"
              << "=== Unpack directory: " << dir << '\n'
              << "=== Unpack archiveName: " << pak << '\n'
              << "===================================================\n";

    fs->CreateDirectory(dir);

    std::unique_ptr<ResourceArchive, Deleter<ResourceArchive>> ra(
        new ResourceArchive(), [](ResourceArchive* ptr)
        {
            SafeRelease(ptr);
        });
    ra->Open(pathArchiveNameDir);

    // TODO
    return EXIT_SUCCESS;
}

ResourceArchive::CompressionType ToPackType(
    const String& value,
    ResourceArchive::CompressionType defaultVal)
{
    const Vector<std::pair<String, ResourceArchive::CompressionType>>
        packTypes = {{"lz4", ResourceArchive::CompressionType::Lz4},
                     {"lz4hc", ResourceArchive::CompressionType::Lz4HC},
                     {"none", ResourceArchive::CompressionType::None}};
    for (auto& pair : packTypes)
    {
        if (pair.first == value)
        {
            return pair.second;
        }
    }
    Logger::Error(
        "can't convert: \"%s\" into any valid compression type, use default\n",
        value.c_str());
    return defaultVal;
}

int main(int argc, char* argv[])
{
    auto result = EXIT_FAILURE;

    ProgramOptions packOptions("pack");
    packOptions.AddOption("--compression", VariantType(String("lz4hc")),
                          "default compression method, lz4hc - default");
    // dafault rule pack all files into lz4hc
    packOptions.AddOption("--rule", VariantType(String(".lz4hc")),
                          "rule to select compression type like: --rule "
                          "xml.lz4 suported lz4, lz4hc, none",
                          true);
    packOptions.AddArgument("directory");
    packOptions.AddArgument("pakfile");

    ProgramOptions unpackOptions("unpack");
    unpackOptions.AddArgument("directory");
    unpackOptions.AddArgument("pakfile");

    if (packOptions.Parse(argc, argv))
    {
        auto compression = packOptions.GetOption("--compression").AsString();

        ResourceArchive::Rules compressionRules;

        for (uint32 i = 0; i < packOptions.GetOptionsCount("--rule"); ++i)
        {
            VariantType option = packOptions.GetOption("--rule", i);
            String str = option.AsString();
            size_t dotPos = str.find('.');
            if (dotPos == String::npos)
            {
                Logger::Error("incorrect option: %s\n", str.c_str());
                return EXIT_FAILURE;
            }
            String ext = str.substr(0, dotPos);
            String compressionType = str.substr(dotPos + 1);
            ResourceArchive::CompressionType packType =
                ToPackType(compressionType, ResourceArchive::CompressionType::Lz4HC);
            compressionRules.push_back({ext, packType});
        }

        auto dirName = packOptions.GetArgument("directory");
        auto pakFile = packOptions.GetArgument("pakfile");

        result = PackDirectoryIntoPakfile(dirName, pakFile, compressionRules);
    }
    else if (unpackOptions.Parse(argc, argv))
    {
        auto pakFile = unpackOptions.GetArgument("pakfile");
        auto dirName = unpackOptions.GetArgument("directory");

        result = UnpackPackfileIntoDirectory(pakFile, dirName);
    }
    else
    {
        packOptions.PrintUsage();
        unpackOptions.PrintUsage();
    }

    return result;
}
