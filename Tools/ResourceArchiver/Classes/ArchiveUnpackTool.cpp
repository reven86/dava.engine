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
#include "FileSystem/FileList.h"
#include "ResourceArchiver/ResourceArchiver.h"

#include "ArchiveUnpackTool.h"

using namespace DAVA;

ArchiveUnpackTool::ArchiveUnpackTool()
    : CommandLineTool("-unpack")
{
    options.AddArgument("packfile");
    options.AddArgument("directory");
}

bool ArchiveUnpackTool::ConvertOptionsToParamsInternal()
{
    packFilename = options.GetArgument("packfile");
    if (packFilename.IsEmpty())
    {
        LOG_ERROR("packfile param is not specified");
        return false;
    }

    dstDir = options.GetArgument("directory");
    dstDir.MakeDirectoryPathname();
    if (dstDir.IsEmpty())
    {
        LOG_ERROR("directory param is not specified");
        return false;
    }

    return true;
}

void ArchiveUnpackTool::ProcessInternal()
{
    Logger::Info("Unpacking %s into %s", packFilename.GetFilename().c_str(), dstDir.GetAbsolutePathname().c_str());

    FilePath currentDir = FileSystem::Instance()->GetCurrentWorkingDirectory();
    SCOPE_EXIT
    {
        FileSystem::Instance()->SetCurrentWorkingDirectory(currentDir);
    };

    if (FileSystem::Instance()->CreateDirectory(dstDir) == FileSystem::DIRECTORY_CANT_CREATE)
    {
        LOG_ERROR("Can't create dir '%s'", dstDir.GetAbsolutePathname().c_str());
        return;
    }

    if (FileSystem::Instance()->SetCurrentWorkingDirectory(dstDir) == false)
    {
        LOG_ERROR("Can't change current dir to '%s'", dstDir.GetAbsolutePathname().c_str());
        return;
    }

    try
    {
        ResourceArchive resourceArchive(packFilename);

        for (const ResourceArchive::FileInfo& fileInfo : resourceArchive.GetFilesInfo())
        {
            Logger::Info("Unpacking %s, compressed size %u, orig size %u, pack type %s",
                         fileInfo.relativeFilePath.c_str(), fileInfo.compressedSize, fileInfo.originalSize,
                         ResourceArchiver::CompressTypeToString(fileInfo.compressionType).c_str());
            if (!UnpackFile(resourceArchive, fileInfo))
            {
                return;
            }
        }
        Logger::Info("done");
    }
    catch (std::exception ex)
    {
        LOG_ERROR("Can't open archive %s: %s", packFilename.GetAbsolutePathname().c_str(), ex.what());
    }
}

bool ArchiveUnpackTool::UnpackFile(const ResourceArchive& archive, const ResourceArchive::FileInfo& fileInfo)
{
    Vector<uint8> content;
    if (!archive.LoadFile(fileInfo.relativeFilePath, content))
    {
        LOG_ERROR("Can't load file %s from archive", fileInfo.relativeFilePath);
        return false;
    }

    FilePath fullPath(fileInfo.relativeFilePath);
    FilePath dirPath = fullPath.GetDirectory();
    FileSystem::eCreateDirectoryResult result = FileSystem::Instance()->CreateDirectory(dirPath, true);
    if (FileSystem::DIRECTORY_CANT_CREATE == result)
    {
        LOG_ERROR("Can't create unpack path dir %s", dirPath.GetAbsolutePathname().c_str());
        return false;
    }

    if (!fullPath.IsDirectoryPathname())
    {
        UniquePtr<File> file(File::Create(fullPath, File::CREATE | File::WRITE));
        if (!file)
        {
            LOG_ERROR("Can't create file %s", fullPath.GetAbsolutePathname().c_str());
            return false;
        }

        uint32 dataSize = static_cast<uint32>(content.size());
        uint32 written = file->Write(content.data(), dataSize);
        if (written != dataSize)
        {
            LOG_ERROR("Can't write into %s", fullPath.GetAbsolutePathname().c_str());
            return false;
        }
    }

    return true;
}
