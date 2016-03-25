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

#include "ArchiveUnpackTool.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileList.h"
#include "Utils.h"

using namespace DAVA;

ArchiveUnpackTool::ArchiveUnpackTool()
    : CommandLineTool("-unpack")
{
    options.AddArgument("directory");
    options.AddArgument("pakfile");
}

bool ArchiveUnpackTool::ConvertOptionsToParamsInternal()
{
    packFilename = options.GetArgument("pakfile");
    if (packFilename.empty())
    {
        Logger::Error("pakfile param is not specified");
        return false;
    }

    dstDir = options.GetArgument("directory");
    dstDir.MakeDirectoryPathname();
    if (dstDir.IsEmpty())
    {
        Logger::Error("directory param is not specified");
        return false;
    }

    return true;
}

void ArchiveUnpackTool::ProcessInternal()
{
    Logger::Info("Unpacking %s into %s", packFilename.c_str(), dstDir.GetAbsolutePathname().c_str());

    FilePath currentDir = FileSystem::Instance()->GetCurrentWorkingDirectory();
    SCOPE_EXIT
    {
        FileSystem::Instance()->SetCurrentWorkingDirectory(currentDir);
    };

    if (FileSystem::Instance()->CreateDirectory(dstDir) == FileSystem::DIRECTORY_CANT_CREATE)
    {
        Logger::Error("Can't create dir '%s'", dstDir.GetAbsolutePathname().c_str());
        return;
    }

    if (FileSystem::Instance()->SetCurrentWorkingDirectory(dstDir) == false)
    {
        Logger::Error("Can't change current dir to '%s'", dstDir.GetAbsolutePathname().c_str());
        return;
    }

    FilePath packFullPath = currentDir + packFilename;
    std::unique_ptr<ResourceArchive> resourceArchive(new ResourceArchive(packFullPath));

    if (!resourceArchive)
    {
        Logger::Error("Can't open archive %s", packFullPath.GetAbsolutePathname().c_str());
        return;
    }

    for (auto& fileInfo : resourceArchive->GetFilesInfo())
    {
        Logger::Info("Unpacking %s, compressed size %d, orig size %d, pack type %s",
                     fileInfo.fileName, fileInfo.compressedSize, fileInfo.originalSize,
                     ResourceArchiveToolUtils::ToString(fileInfo.compressionType));
        if (!UnpackFile(*resourceArchive, fileInfo))
        {
            return;
        }
    }
    Logger::Info("Success");
}

bool ArchiveUnpackTool::UnpackFile(const ResourceArchive& ra, const ResourceArchive::FileInfo& fileInfo)
{
    Vector<uint8> content;
    if (!ra.LoadFile(fileInfo.fileName, content))
    {
        Logger::Error("Can't load file %s from archive", fileInfo.fileName);
        return false;
    }
    String filePath(fileInfo.fileName);
    FilePath fullPath = filePath;
    FilePath dirPath = fullPath.GetDirectory();
    FileSystem::eCreateDirectoryResult result = FileSystem::Instance()->CreateDirectory(dirPath, true);
    if (FileSystem::DIRECTORY_CANT_CREATE == result)
    {
        Logger::Error("Can't create unpack path dir %s", dirPath.GetAbsolutePathname().c_str());
        return false;
    }
    ScopedPtr<File> f(File::Create(fullPath, File::CREATE | File::WRITE));
    if (!f)
    {
        Logger::Error("Can't create file %s", fileInfo.fileName);
        return false;
    }
    uint32 written = f->Write(content.data(), content.size());
    if (written != content.size())
    {
        Logger::Error("Can't write into %s", fileInfo.fileName);
        return false;
    }
    return true;
}
