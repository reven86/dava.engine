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


#include "FileSystem/APKFile.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "FileSystem/ResourceArchive.h"
#include "Platform/Mutex.h"

#include <android/asset_manager.h>

namespace DAVA
{

Mutex APKFile::mutex = Mutex();

APKFile::APKFile()
    : DynamicMemoryFile()
{
    
}

APKFile::~APKFile()
{
    
}

File * APKFile::CreateFromAssets(const FilePath &filePath, uint32 attributes)
{
	mutex.Lock();
//	Logger::Debug("[APKFile::CreateFromAssets] wan't to create file %s", filePath.c_str());
//
    FileSystem * fileSystem = FileSystem::Instance();
	for (List<FileSystem::ResourceArchiveItem>::iterator ai = fileSystem->resourceArchiveList.begin();
         ai != fileSystem->resourceArchiveList.end(); ++ai)
	{
		FileSystem::ResourceArchiveItem & item = *ai;
        
		String filenamecpp = filePath.GetAbsolutePathname();
        
		String::size_type pos = filenamecpp.find(item.attachPath);
		if (pos == 0)
		{
			String relfilename = filenamecpp.substr(item.attachPath.length());
			int32 size = item.archive->LoadResource(relfilename, 0);
			if ( size == -1 )
			{
				mutex.Unlock();
				return 0;
			}
            
			uint8 * buffer = new uint8[size];
			item.archive->LoadResource(relfilename, buffer);

            APKFile *fileInstance = CreateFromData(relfilename, buffer, size, attributes);
            SafeDeleteArray(buffer);
            mutex.Unlock();
			return fileInstance;
		}
	}
    
    bool isDirectory = FileSystem::Instance()->IsDirectory(filePath);
    if(isDirectory)
    {
        Logger::Error("[APKFile::CreateFromAssets] Can't create file because of it is directory (%s)", filePath.GetAbsolutePathname().c_str());
        mutex.Unlock();
        return NULL;
    }
    
    
    CorePlatformAndroid *core = (CorePlatformAndroid *)Core::Instance();
    DVASSERT_MSG(core, "Need create core before loading of files");
    
    
    AAssetManager *assetManager = core->GetAssetManager();
    DVASSERT_MSG(assetManager, "Need setup assetManager on core creation");
    
    AAsset * asset = AAssetManager_open(assetManager, filePath.GetAbsolutePathname().c_str(), AASSET_MODE_UNKNOWN);
    if(!asset)
    {
        Logger::Error("[APKFile::CreateFromAssets] Can't load asset for path %s", filePath.GetAbsolutePathname().c_str());
        mutex.Unlock();
        return NULL;
    }
    

    int32 dataSize = AAsset_getLength(asset);
//    Logger::Debug("[APKFile::CreateFromAssets] fileSize is %d (%s)", dataSize, filePath.c_str());

    uint8 *data = new uint8[dataSize];
    
    int32 readSize = AAsset_read(asset, data, dataSize * sizeof(uint8));
    AAsset_close(asset);

    if (readSize != dataSize)
    {
    	Logger::Error("IsMaiThread:%d, readSize:%d, dataSize:%d",
    		Thread::IsMainThread() ? 1 : 0,
    		readSize,
    		dataSize);
    }

    APKFile *fileInstance = NULL;

    if (readSize < 0)
    {
    	Logger::Error("Error reading %s.", filePath.GetAbsolutePathname().c_str());
    	Logger::Error("This seems to be a known Android issue with resources packing.");
    	Logger::Error("In the case when there are many repetitive values in the file, Android unpacker can't determine the end of the packed file.");
    	Logger::Error("For more information: https://code.google.com/p/android/issues/detail?id=39041 https://android-review.googlesource.com/#/c/51757/");
    	Logger::Error("This issue is fixed in the later versions of Android.");
    	Logger::Error("The workaround is to fill the file with data to avoid the big amount of repetitive values at the end of compressed file.");
    }
    else
    {
    	fileInstance = CreateFromData(filePath, data, readSize, attributes);
    	DVASSERT_MSG(fileInstance, "Can't create dynamic file from memory");
    }

    SafeDeleteArray(data);
    mutex.Unlock();
    return fileInstance;
}
    
APKFile * APKFile::CreateFromData(const FilePath &filePath, const uint8 * data, int32 dataSize, uint32 attributes)
{
    APKFile *fl = new APKFile();
	fl->filename = filePath;
	fl->Write(data, dataSize);
	fl->fileAttributes = attributes;
	fl->currentPtr = 0;

//	Logger::Debug("[APKFile::CreateFromData] fileSize is %d (%s) (%p) ", dataSize, filePath.c_str(), fl);
    
    return fl;
}

    

    
};

#endif // __DAVAENGINE_ANDROID__



