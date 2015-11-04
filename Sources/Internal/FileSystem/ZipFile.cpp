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


#include "FileSystem/ZipFile.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "FileSystem/FileSystem.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "FileSystem/ResourceArchive.h"
#include "FileSystem/FilePath.h"
#include "Platform/TemplateAndroid/AssetsManagerAndroid.h"
#include "Concurrency/LockGuard.h"

namespace DAVA
{

Mutex ZipFile::mutex;

#ifdef USE_LOCAL_RESOURCES
zip* ZipFile::exZipPackage = NULL;
String ZipFile::zipFileName = "Data.zip";
#endif

ZipFile::ZipFile()
    : DynamicMemoryFile()
{
    
}

ZipFile::~ZipFile()
{
}

File * ZipFile::CreateFromAPK(const FilePath &filePath, uint32 attributes)
{
    LockGuard<Mutex> guard(mutex);

    FileSystem * fileSystem = FileSystem::Instance();
	for (List<FileSystem::ResourceArchiveItem>::iterator ai = fileSystem->resourceArchiveList.begin();
         ai != fileSystem->resourceArchiveList.end(); ++ai)
	{
		FileSystem::ResourceArchiveItem & item = *ai;
        
		String filenamecpp = filePath.GetAbsolutePathname();
        
		String::size_type pos = filenamecpp.find(item.attachPath);
		if (0 == pos)
		{
			String relfilename = filenamecpp.substr(item.attachPath.length());
			int32 size = item.archive->LoadResource(relfilename, 0);
			if (-1 == size)
			{
				return 0;
			}
            
			uint8 * buffer = new uint8[size];
			item.archive->LoadResource(relfilename, buffer);

			ZipFile *fileInstance = CreateFromData(relfilename, buffer, size, attributes);
            SafeDeleteArray(buffer);
			return fileInstance;
		}
	}
    
    bool isDirectory = FileSystem::Instance()->IsDirectory(filePath);
    if(isDirectory)
    {
        //Logger::FrameworkDebug("[ZipFile::CreateFromAssets] Can't create file because it is a directory (%s)", filePath.GetAbsolutePathname().c_str());
        return NULL;
    }
    
    AssetsManager* assetsManager = AssetsManager::Instance();
    DVASSERT_MSG(assetsManager, "[ZipFile::CreateFromAssets] Need to create AssetsManager before loading files");

    zip* package = assetsManager->GetApplicationPackage();
    if (NULL == package)
    {
        DVASSERT_MSG(false, "[ZipFile::CreateFromAssets] Package file should be initialized.");
        return NULL;
    }

    String assetFileStr = "assets/" + filePath.GetAbsolutePathname();
    return CreateFromPath(package, filePath, assetFileStr, attributes);
}

#ifdef USE_LOCAL_RESOURCES
File * ZipFile::CreateFromZip(const FilePath &filePath, uint32 attributes)
{
	if (!exZipPackage)
	{
		int32 res = 0;
		String zipPath(localResourcesPath);
		zipPath += zipFileName;
		exZipPackage = zip_open(zipPath.c_str(), ZIP_CHECKCONS, &res);
		if (NULL == exZipPackage)
		{
			//DVASSERT_MSG(false, "[ZipFile::CreateFromZip] Can't initialize zip package.");
			return NULL;
		}
	}

	String path = filePath.GetAbsolutePathname();
	return CreateFromPath(exZipPackage, filePath, path, attributes);
}

void ZipFile::SetZipFileName(const String& fileName)
{
	if (exZipPackage)
	{
		zip_close(exZipPackage);
		exZipPackage = NULL;
	}
	zipFileName = fileName;
}

#endif

ZipFile* ZipFile::CreateFromPath(zip* package, const FilePath &filePath, const String &path, uint32 attributes)
{
    int index = zip_name_locate(package, path.c_str(), 0);
    if (-1 == index)
    {
        //Logger::Error("[ZipFile::CreateFromAssets] Can't locate file in the archive: %s", path.c_str());
        return NULL;
    }

    struct zip_stat stat;

    int32 error = zip_stat_index(package, index, 0, &stat);
    if (-1 == error)
    {
        Logger::FrameworkDebug("[ZipFile::CreateFromAssets] Can't get file info: %s", path.c_str());
        return NULL;
    }

    zip_file* file = zip_fopen_index(package, index, 0);
    if (NULL == file)
    {
        Logger::FrameworkDebug("[ZipFile::CreateFromAssets] Can't open file in the archive: %s", path.c_str());
        return NULL;
    }

    DVASSERT(stat.size >= 0);
    uint8 *data = new uint8[stat.size];

    if (zip_fread(file, data, stat.size) != stat.size)
    {
        Logger::FrameworkDebug("[ZipFile::CreateFromAssets] Error reading file: %s", path.c_str());
        SafeDeleteArray(data);
        zip_fclose(file);
        return NULL;
    }

    ZipFile *fileInstance = CreateFromData(filePath, data, stat.size, attributes);
    DVASSERT_MSG(fileInstance, "[ZipFile::CreateFromAssets] Can't create dynamic file from memory");

    SafeDeleteArray(data);
    zip_fclose(file);
    return fileInstance;
}
    
ZipFile * ZipFile::CreateFromData(const FilePath &filePath, const uint8 * data, int32 dataSize, uint32 attributes)
{
	ZipFile *fl = new ZipFile();
	fl->filename = filePath;
	fl->Write(data, dataSize);
	fl->fileAttributes = attributes;
	fl->currentPtr = 0;
    
    return fl;
}

    

    
};

#endif // __DAVAENGINE_ANDROID__



