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


#include "FileSystem/FileList.h"
#include "Utils/Utils.h"
#include "Utils/Utf8Utils.h"

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#elif defined(__DAVAENGINE_WIN32__)
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <direct.h>
#elif defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/FileListAndroid.h"
#endif //PLATFORMS

namespace DAVA
{

FileList::FileList(const FilePath & filepath)
{
    DVASSERT(filepath.IsDirectoryPathname());
    
	path = filepath;

// Windows Store version doesnt exist
#if defined(__DAVAENGINE_WINDOWS_STORE__)
    __DAVAENGINE_WINDOWS_STORE_INCOMPLETE_IMPLEMENTATION__
    return;

// Windows version
#elif defined(__DAVAENGINE_WINDOWS_DESKTOP__)

	//char tmp[_MAX_PATH];
	//_getcwd(tmp, _MAX_PATH);
	//Path = tmp;
	FilePath prevDir = FileSystem::Instance()->GetCurrentWorkingDirectory();
	BOOL res = SetCurrentDirectoryApath.GetAbsolutePathname().c_str());

	if (res)
	{
		struct _finddata_t c_file;
		intptr_t hFile;
		FileEntry entry;

		if( (hFile = _findfirst( "*", &c_file )) != -1L )
		{
			do
			{
                //TODO: need to check for Win32
				entry.path = filepath + c_file.name;
				entry.name = c_file.name;
				entry.size = c_file.size;
				entry.isDirectory = (_A_SUBDIR & c_file.attrib) != 0;
				if(entry.isDirectory)
				{
					entry.path.MakeDirectoryPathname();
				}

				fileList.push_back(entry);
				//Logger::FrameworkDebug("filelist: %s %s", filepath.c_str(), entry.name.c_str());
			}
			while( _findnext( hFile, &c_file ) == 0 );

			_findclose( hFile );
		}
	}
	FileSystem::Instance()->SetCurrentWorkingDirectory(prevDir);

	//TODO add drives
	//entry.Name = "E:\\";
	//entry.isDirectory = true;
	//Files.push_back(entry);
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
	struct dirent **namelist;
	FileEntry entry;

	int32 n = scandir(path.GetAbsolutePathname().c_str(), &namelist, 0, alphasort);

	if (n >= 0)
	{
		while(n--)
		{
			entry.path = path + namelist[n]->d_name;
			entry.name = namelist[n]->d_name;
			entry.size = 0;
			entry.isDirectory = namelist[n]->d_type == DT_DIR;
            if(entry.isDirectory)
            {
                entry.path.MakeDirectoryPathname();
            }
			fileList.push_back(entry);
			free(namelist[n]);
		}
		free(namelist);
	}
#elif defined (__DAVAENGINE_ANDROID__)
	JniFileList jniFileList;
	Vector<JniFileList::JniFileListEntry> entrys = jniFileList.GetFileList(path.GetAbsolutePathname());
	FileEntry entry;
	for (int32 i = 0; i < entrys.size(); ++i)
	{
		const JniFileList::JniFileListEntry& jniEntry = entrys[i];

		entry.path = path + jniEntry.name;
		entry.name = jniEntry.name;
		entry.size = jniEntry.size;
		entry.isDirectory = jniEntry.isDirectory;

		if(entry.isDirectory)
		{
			entry.path.MakeDirectoryPathname();
		}
		fileList.push_back(entry);
	}
#endif //PLATFORMS

	directoryCount = 0;
	fileCount = 0;
	for (int fi = 0; fi < GetCount(); ++fi)
	{
		if (IsDirectory(fi))
		{
			if (!IsNavigationDirectory(fi))
				directoryCount++;
		}else
			fileCount++;
	}
}

FileList::~FileList()
{

}

int32 FileList::GetCount() const
{
	return (int32)fileList.size();
}	

int32 FileList::GetFileCount() const
{
	return fileCount;
}

int32 FileList::GetDirectoryCount() const
{
	return directoryCount;
}

const FilePath & FileList::GetPathname(int32 index) const
{
	DVASSERT((index >= 0) && (index < (int32)fileList.size()));
	return fileList[index].path;
}
    
const String & FileList::GetFilename(int32 index) const
{
    DVASSERT((index >= 0) && (index < (int32)fileList.size()));
    return fileList[index].name;
}
    

bool FileList::IsDirectory(int32 index)  const
{
	DVASSERT((index >= 0) && (index < (int32)fileList.size()));
	return fileList[index].isDirectory;
}
	
bool FileList::IsNavigationDirectory(int32 index) const
{
	DVASSERT((index >= 0) && (index < (int32)fileList.size()));
	//bool isDir = fileList[index].isDirectory;
	//if (isDir)
	//{
    
    String filename = GetFilename(index);
	if ((filename == ".") || (filename == ".."))return true;
	//}
	return false;
}


//bool FileList::FileEntry::operator< (const FileList::FileEntry &other)
//{
//    if (!isDirectory && other.isDirectory) 
//    {
//        return true;
//    }
//    
//    if (name < other.name)
//    {
//        return true;
//    }
//    
//    return false;
//}

void FileList::Sort()
{
    std::sort(fileList.begin(), fileList.end());
}
    
    
}; // end of namespace DAVA
