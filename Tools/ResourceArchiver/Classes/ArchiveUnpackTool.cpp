#include "Base/GlobalEnum.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileList.h"
#include "ResourceArchiver/ResourceArchiver.h"

#include "ArchiveUnpackTool.h"
#include "ResultCodes.h"

using namespace DAVA;

ArchiveUnpackTool::ArchiveUnpackTool()
    : CommandLineTool("unpack")
{
    options.AddArgument("packfile");
    options.AddArgument("directory");
}

bool ArchiveUnpackTool::ConvertOptionsToParamsInternal()
{
    packFilename = options.GetArgument("packfile");
    if (packFilename.IsEmpty())
    {
        Logger::Error("packfile param is not specified");
        return false;
    }

    dstDir = options.GetArgument("directory");
    if (dstDir.IsEmpty())
    {
        Logger::Error("directory param is not specified");
        return false;
    }
    dstDir.MakeDirectoryPathname();

    return true;
}

int ArchiveUnpackTool::ProcessInternal()
{
    Logger::Info("Unpacking %s into %s", packFilename.GetFilename().c_str(), dstDir.GetAbsolutePathname().c_str());

    FilePath currentDir = FileSystem::Instance()->GetCurrentWorkingDirectory();
    SCOPE_EXIT
    {
        FileSystem::Instance()->SetCurrentWorkingDirectory(currentDir);
    };

    if (FileSystem::Instance()->CreateDirectory(dstDir) == FileSystem::DIRECTORY_CANT_CREATE)
    {
        Logger::Error("Can't create dir '%s'", dstDir.GetAbsolutePathname().c_str());
        return ResourceArchiverResult::ERROR_CANT_CREATE_DIR;
    }

    if (FileSystem::Instance()->SetCurrentWorkingDirectory(dstDir) == false)
    {
        Logger::Error("Can't change current dir to '%s'", dstDir.GetAbsolutePathname().c_str());
        return ResourceArchiverResult::ERROR_CANT_CHANGE_DIR;
    }

    try
    {
        ResourceArchive resourceArchive(packFilename);

        int unpackResult = ERROR_EMPTY_ARCHIVE;
        for (const ResourceArchive::FileInfo& fileInfo : resourceArchive.GetFilesInfo())
        {
            Logger::Info("Unpacking %s, compressed size %u, orig size %u, pack type %s",
                         fileInfo.relativeFilePath.c_str(), fileInfo.compressedSize, fileInfo.originalSize,
                         GlobalEnumMap<Compressor::Type>::Instance()->ToString(static_cast<int>(fileInfo.compressionType)));

            unpackResult = UnpackFile(resourceArchive, fileInfo);
            if (unpackResult != ResourceArchiverResult::OK)
                break;
        }

        return unpackResult;
    }
    catch (std::exception& ex)
    {
        Logger::Error("Can't open archive %s: %s", packFilename.GetAbsolutePathname().c_str(), ex.what());
        return ResourceArchiverResult::ERROR_CANT_OPEN_ARCHIVE;
    }
}

int ArchiveUnpackTool::UnpackFile(const ResourceArchive& archive, const ResourceArchive::FileInfo& fileInfo)
{
    Vector<uint8> content;
    if (!archive.LoadFile(fileInfo.relativeFilePath, content))
    {
        Logger::Error("Can't load file %s from archive", fileInfo.relativeFilePath.c_str());
        return ResourceArchiverResult::ERROR_CANT_EXTRACT_FILE;
    }

    FilePath fullPath(fileInfo.relativeFilePath);
    FilePath dirPath = fullPath.GetDirectory();
    FileSystem::eCreateDirectoryResult result = FileSystem::Instance()->CreateDirectory(dirPath, true);
    if (FileSystem::DIRECTORY_CANT_CREATE == result)
    {
        Logger::Error("Can't create unpack path dir %s", dirPath.GetAbsolutePathname().c_str());
        return ResourceArchiverResult::ERROR_CANT_CREATE_DIR;
    }

    if (!fullPath.IsDirectoryPathname())
    {
        ScopedPtr<File> file(File::Create(fullPath, File::CREATE | File::WRITE));
        if (!file)
        {
            Logger::Error("Can't create file %s", fullPath.GetAbsolutePathname().c_str());
            return ResourceArchiverResult::ERROR_CANT_WRITE_FILE;
        }

        uint32 dataSize = static_cast<uint32>(content.size());
        uint32 written = file->Write(content.data(), dataSize);
        if (written != dataSize)
        {
            Logger::Error("Can't write into %s", fullPath.GetAbsolutePathname().c_str());
            return ResourceArchiverResult::ERROR_CANT_WRITE_FILE;
        }
    }

    return ResourceArchiverResult::OK;
}
