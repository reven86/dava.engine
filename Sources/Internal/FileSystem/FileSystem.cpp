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
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Utils/StringFormat.h"
#include "FileSystem/ResourceArchive.h"


#if defined(__DAVAENGINE_MACOS__)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <copyfile.h>
#include <libproc.h>
#include <libgen.h>
#elif defined(__DAVAENGINE_IPHONE__)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <copyfile.h>
#include <libgen.h>
#include <sys/sysctl.h>
#elif defined(__DAVAENGINE_WIN32__)
#include <direct.h>
#include <io.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <Shlobj.h>
#include <tchar.h>
#elif defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>

#endif //PLATFORMS

namespace DAVA
{

	

FileSystem::FileSystem()
{
}

FileSystem::~FileSystem()
{	
	for (List<ResourceArchiveItem>::iterator ai = resourceArchiveList.begin();
		ai != resourceArchiveList.end(); ++ai)
	{
		ResourceArchiveItem & item = *ai;
		SafeRelease(item.archive);
	}
	resourceArchiveList.clear();
}

FileSystem::eCreateDirectoryResult FileSystem::CreateDirectory(const FilePath & filePath, bool isRecursive)
{
    DVASSERT(filePath.GetType() != FilePath::PATH_IN_RESOURCES);
    
	if (!isRecursive)
	{
        return CreateExactDirectory(filePath);
	}

    String path = filePath.GetAbsolutePathname();

	Vector<String> tokens;
    Split(path, "/", tokens);
    
	String dir = "";

#if defined (__DAVAENGINE_WIN32__)
    if(0 < tokens.size() && 0 < tokens[0].length())
    {
        String::size_type pos = path.find(tokens[0]);
        if(String::npos != pos)
        {
            tokens[0] = path.substr(0, pos) + tokens[0];
        }
    }
#else //#if defined (__DAVAENGINE_WIN32__)
    String::size_type find = path.find(":");
    if(find == String::npos)
	{
        dir = "/";
    }
#endif //#if defined (__DAVAENGINE_WIN32__)
	
	for (size_t k = 0; k < tokens.size(); ++k)
	{
		dir += tokens[k] + "/";
        
        eCreateDirectoryResult ret = CreateExactDirectory(dir);
		if (k == tokens.size() - 1)
        {
            return ret;
        }
	}
	return DIRECTORY_CANT_CREATE;
}
    
FileSystem::eCreateDirectoryResult FileSystem::CreateExactDirectory(const FilePath & filePath)
{
    DVASSERT(filePath.GetType() != FilePath::PATH_IN_RESOURCES);

    if(IsDirectory(filePath))
        return DIRECTORY_EXISTS;
    
#ifdef __DAVAENGINE_WIN32__
    BOOL res = ::CreateDirectoryA(filePath.GetAbsolutePathname().c_str(), 0);
    return (res == 0) ? DIRECTORY_CANT_CREATE : DIRECTORY_CREATED;
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    int res = mkdir(filePath.GetAbsolutePathname().c_str(), 0777);
    return (res == 0) ? (DIRECTORY_CREATED) : (DIRECTORY_CANT_CREATE);
#endif //PLATFORMS
}


bool FileSystem::CopyFile(const FilePath & existingFile, const FilePath & newFile, bool overwriteExisting /* = false */)
{
    DVASSERT(newFile.GetType() != FilePath::PATH_IN_RESOURCES);

#ifdef __DAVAENGINE_WIN32__
	BOOL ret = ::CopyFileA(existingFile.GetAbsolutePathname().c_str(), newFile.GetAbsolutePathname().c_str(), !overwriteExisting);
	return ret != 0;
#elif defined(__DAVAENGINE_ANDROID__)

	bool copied = false;

	File *srcFile = File::Create(existingFile, File::OPEN | File::READ);
	File *dstFile = File::Create(newFile, File::WRITE | File::CREATE);
	if(srcFile && dstFile)
	{
		uint32 fileSize = srcFile->GetSize();
        uint8 *data = new uint8[fileSize];
        if(data)
        {
			uint32 read = srcFile->Read(data, fileSize);
            if(read == fileSize)
            {
                uint32 written = dstFile->Write(data, fileSize);
                if(written == fileSize)
                {
                    copied = true;
                }
                else
                {
                    Logger::Error("[FileSystem::CopyFile] can't write to file %s", newFile.GetAbsolutePathname().c_str());
                }
            }
            else
            {
                Logger::Error("[FileSystem::CopyFile] can't read file %s", existingFile.GetAbsolutePathname().c_str());
            }
            
            SafeDeleteArray(data);
        }
        else
        {
            Logger::Error("[FileSystem::CopyFile] can't allocate memory of %d Bytes", fileSize);
        }
	}

	SafeRelease(dstFile);
	SafeRelease(srcFile);

	return copied;

#else //iphone & macos
    int ret = copyfile(existingFile.GetAbsolutePathname().c_str(), newFile.GetAbsolutePathname().c_str(), NULL, overwriteExisting ? COPYFILE_ALL : COPYFILE_ALL | COPYFILE_EXCL);
    return ret==0;
#endif //PLATFORMS
}

bool FileSystem::MoveFile(const FilePath & existingFile, const FilePath & newFile, bool overwriteExisting/* = false*/)
{
    DVASSERT(newFile.GetType() != FilePath::PATH_IN_RESOURCES);

#ifdef __DAVAENGINE_WIN32__
	DWORD flags = (overwriteExisting) ? MOVEFILE_REPLACE_EXISTING : 0;
	BOOL ret = ::MoveFileExA(existingFile.GetAbsolutePathname().c_str(), newFile.GetAbsolutePathname().c_str(), flags);
	return ret != 0;
#elif defined(__DAVAENGINE_ANDROID__)
	if (!overwriteExisting && access(newFile.GetAbsolutePathname().c_str(), 0) != -1)
	{
		return false;
	}
	remove(newFile.GetAbsolutePathname().c_str());
	int ret = rename(existingFile.GetAbsolutePathname().c_str(), newFile.GetAbsolutePathname().c_str());
	return ret == 0;
#else //iphone & macos
	int flags = COPYFILE_ALL | COPYFILE_MOVE;
	if(!overwriteExisting)
		flags |= COPYFILE_EXCL;
	
	int ret = copyfile(existingFile.GetAbsolutePathname().c_str(), newFile.GetAbsolutePathname().c_str(), NULL, flags);
	return ret==0;
#endif //PLATFORMS
}


bool FileSystem::CopyDirectory(const FilePath & sourceDirectory, const FilePath & destinationDirectory, bool overwriteExisting /* = false */)
{
    DVASSERT(destinationDirectory.GetType() != FilePath::PATH_IN_RESOURCES);
    DVASSERT(sourceDirectory.IsDirectoryPathname() && destinationDirectory.IsDirectoryPathname());
    
	bool ret = true;

	ScopedPtr<FileList> fileList( new FileList(sourceDirectory));
	int32 count = fileList->GetCount();
	String fileOnly;
	String pathOnly;
	for(int32 i = 0; i < count; ++i)
	{
		if(!fileList->IsDirectory(i) && !fileList->IsNavigationDirectory(i))
		{
            const FilePath destinationPath = destinationDirectory + fileList->GetFilename(i);
			if(!CopyFile(fileList->GetPathname(i), destinationPath, overwriteExisting))
			{
				ret = false;
			}
		}
	}

	return ret;
}
	
bool FileSystem::DeleteFile(const FilePath & filePath)
{
    DVASSERT(filePath.GetType() != FilePath::PATH_IN_RESOURCES);

	// function unlink return 0 on success, -1 on error
	int res = remove(filePath.GetAbsolutePathname().c_str());
	return (res == 0);
}
	
bool FileSystem::DeleteDirectory(const FilePath & path, bool isRecursive)
{
    DVASSERT(path.GetType() != FilePath::PATH_IN_RESOURCES);
    DVASSERT(path.IsDirectoryPathname());
    
	FileList * fileList = new FileList(path);
	for(int i = 0; i < fileList->GetCount(); ++i)
	{
		if(fileList->IsDirectory(i))
		{
			if(!fileList->IsNavigationDirectory(i))
			{
				if(isRecursive)
				{
//					Logger::FrameworkDebug("- try to delete directory: %s / %s", fileList->GetPathname(i).c_str(), fileList->GetFilename(i).c_str());
					bool success = DeleteDirectory(fileList->GetPathname(i), isRecursive);
//					Logger::FrameworkDebug("- delete directory: %s / %s- %d", fileList->GetPathname(i).c_str(), fileList->GetFilename(i).c_str(), success ? (1): (0));
					if (!success)return false;
				}
			}
		}
		else 
		{
			bool success = DeleteFile(fileList->GetPathname(i));
//			Logger::FrameworkDebug("- delete file: %s / %s- %d", fileList->GetPathname(i).c_str(), fileList->GetFilename(i).c_str(), success ? (1): (0));
			if(!success)return false;
		}
	}
	SafeRelease(fileList);
#ifdef __DAVAENGINE_WIN32__
	String sysPath = path.GetAbsolutePathname();
	int32 chmodres = _chmod(sysPath.c_str(), _S_IWRITE); // change read-only file mode
	int32 res = _rmdir(sysPath.c_str());
	return (res == 0);
	/*int32 res = ::RemoveDirectoryA(path.c_str());
	if (res == 0)
	{
		Logger::Warning("Failed to delete directory: %s error: 0x%x", path.c_str(), GetLastError());
	}
	return (res != 0);*/
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	int32 res = rmdir(path.GetAbsolutePathname().c_str());
	return (res == 0);
#endif //PLATFORMS
}
	
uint32 FileSystem::DeleteDirectoryFiles(const FilePath & path, bool isRecursive)
{
    DVASSERT(path.GetType() != FilePath::PATH_IN_RESOURCES);
    DVASSERT(path.IsDirectoryPathname());

	uint32 fileCount = 0;
	
	FileList * fileList = new FileList(path);
	for(int i = 0; i < fileList->GetCount(); ++i)
	{
		if(fileList->IsDirectory(i))
		{
			if(!fileList->IsNavigationDirectory(i))
			{
				if(isRecursive)
				{
					fileCount += DeleteDirectoryFiles(fileList->GetPathname(i), isRecursive);
				}
			}
		}
		else 
		{
			bool success = DeleteFile(fileList->GetPathname(i));
			if(success)fileCount++;
		}
	}
	SafeRelease(fileList);

	return fileCount;
}


	
File *FileSystem::CreateFileForFrameworkPath(const FilePath & frameworkPath, uint32 attributes)
{
#if defined(__DAVAENGINE_ANDROID__)
    if (frameworkPath.GetType() == FilePath::PATH_IN_RESOURCES &&
        frameworkPath.GetAbsolutePathname().size() &&
        frameworkPath.GetAbsolutePathname().c_str()[0] != '/')
    {
#ifdef USE_LOCAL_RESOURCES
        return File::CreateFromSystemPath(frameworkPath, attributes);
#else
        return APKFile::CreateFromAssets(frameworkPath, attributes);
#endif
    }
    else
    {
        return File::CreateFromSystemPath(frameworkPath, attributes);
    }
#else //#if defined(__DAVAENGINE_ANDROID__)
	return File::CreateFromSystemPath(frameworkPath, attributes);
#endif //#if defined(__DAVAENGINE_ANDROID__)
}


const FilePath & FileSystem::GetCurrentWorkingDirectory()
{
	char tempDir[2048];
#if defined(__DAVAENGINE_WIN32__)
	::GetCurrentDirectoryA(2048, tempDir);
	currentWorkingDirectory = FilePath(tempDir);
	currentWorkingDirectory.MakeDirectoryPathname();
	return currentWorkingDirectory;
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	getcwd(tempDir, 2048);
	currentWorkingDirectory = FilePath(tempDir);
	currentWorkingDirectory.MakeDirectoryPathname();
	return currentWorkingDirectory;
#endif //PLATFORMS
	currentWorkingDirectory.MakeDirectoryPathname();
	return currentWorkingDirectory;
}

FilePath FileSystem::GetCurrentExecutableDirectory()
{
    char tempDir[2048];
    FilePath currentExecuteDirectory;
#if defined(__DAVAENGINE_WIN32__)
    ::GetModuleFileNameA( NULL, tempDir, 2048 );
    currentExecuteDirectory = FilePath(tempDir).GetDirectory();
#elif defined(__DAVAENGINE_MACOS__)
    proc_pidpath(getpid(), tempDir, sizeof(tempDir));
    currentExecuteDirectory = FilePath(dirname(tempDir));
#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	DVASSERT(0);
#endif //PLATFORMS
	currentExecuteDirectory.MakeDirectoryPathname();
	return currentExecuteDirectory;
}

bool FileSystem::SetCurrentWorkingDirectory(const FilePath & newWorkingDirectory)
{
    DVASSERT(newWorkingDirectory.IsDirectoryPathname());
    
#if defined(__DAVAENGINE_WIN32__)
	BOOL res = ::SetCurrentDirectoryA(newWorkingDirectory.GetAbsolutePathname().c_str());
	return (res != 0);
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    
	return (chdir(newWorkingDirectory.GetAbsolutePathname().c_str()) == 0);
#endif //PLATFORMS
	return false; 
}
  
bool FileSystem::IsFile(const FilePath & pathToCheck)
{
#if defined(__DAVAENGINE_ANDROID__)
	const String& path = pathToCheck.GetAbsolutePathname();
	if (IsAPKPath(path))
		return (fileSet.find(path) != fileSet.end());
#endif
	struct stat s;
	if(stat(pathToCheck.GetAbsolutePathname().c_str(),&s) == 0)
	{
		return (0 != (s.st_mode & S_IFREG));
	}

 	return false;
}

bool FileSystem::IsDirectory(const FilePath & pathToCheck)
{
#if defined (__DAVAENGINE_WIN32__)
	DWORD stats = GetFileAttributesA(pathToCheck.GetAbsolutePathname().c_str());
	return (stats != -1) && (0 != (stats & FILE_ATTRIBUTE_DIRECTORY));
#else //defined (__DAVAENGINE_WIN32__)
#if defined(__DAVAENGINE_ANDROID__)
	String path = pathToCheck.GetAbsolutePathname();
	if (path.length() &&
		path.at(path.length() - 1) == '/')
		path.erase(path.begin() + path.length() - 1);
	if (IsAPKPath(path))
		return (dirSet.find(path) != dirSet.end());
#endif //#if defined(__DAVAENGINE_ANDROID__)

	struct stat s;
	if(stat(pathToCheck.GetAbsolutePathname().c_str(), &s) == 0)
	{
		return (0 != (s.st_mode & S_IFDIR));
	}
#endif //#if defined (__DAVAENGINE_WIN32__)

	return false;
}

const FilePath & FileSystem::GetCurrentDocumentsDirectory()
{
    return currentDocDirectory; 
}

void FileSystem::SetCurrentDocumentsDirectory(const FilePath & newDocDirectory)
{
    currentDocDirectory = newDocDirectory;
}

void FileSystem::SetDefaultDocumentsDirectory()
{
    SetCurrentDocumentsDirectory(GetUserDocumentsPath() + "DAVAProject/");
}


#if defined(__DAVAENGINE_WIN32__)
const FilePath FileSystem::GetUserDocumentsPath()
{
    char * szPath = new char[MAX_PATH];
    SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, szPath);
    int32 n = strlen(szPath);
    szPath[n] = '\\';
    szPath[n+1] = 0;
    String str(szPath);
    delete[] szPath;

	FilePath docPath(str);
	docPath.MakeDirectoryPathname();
    return docPath;
}

const FilePath FileSystem::GetPublicDocumentsPath()
{
    char * szPath = new char[MAX_PATH];
    SHGetFolderPathA(NULL, CSIDL_COMMON_DOCUMENTS, NULL, SHGFP_TYPE_CURRENT, szPath);
    int32 n = strlen(szPath);
    szPath[n] = '\\';
    szPath[n+1] = 0;
    String str(szPath);
    delete[] szPath;

	FilePath docPath(str);
	docPath.MakeDirectoryPathname();
	return docPath;
}
#endif //#if defined(__DAVAENGINE_WIN32__)

    
#if defined(__DAVAENGINE_ANDROID__)
const FilePath FileSystem::GetUserDocumentsPath()
{
    CorePlatformAndroid *core = (CorePlatformAndroid *)Core::Instance();
    return core->GetExternalStoragePathname() + String("/");
}

const FilePath FileSystem::GetPublicDocumentsPath()
{
    CorePlatformAndroid *core = (CorePlatformAndroid *)Core::Instance();
    return core->GetExternalStoragePathname() + String("/");
}
#endif //#if defined(__DAVAENGINE_ANDROID__)
    
    
String FileSystem::ReadFileContents(const FilePath & pathname)
{
    File * fp = File::Create(pathname, File::OPEN|File::READ);
	if (!fp)
	{
		Logger::Error("Failed to open file: %s", pathname.GetAbsolutePathname().c_str());
		return 0;
	}
	uint32 fileSize = fp->GetSize();

    String fileContents;
    uint32 dataRead = fp->ReadString(fileContents);
    
	if (dataRead != fileSize)
	{
		Logger::Error("Failed to read data from file: %s", pathname.GetAbsolutePathname().c_str());
		return 0;
	}
    
	SafeRelease(fp);
    return fileContents;
    
}


uint8 * FileSystem::ReadFileContents(const FilePath & pathname, uint32 & fileSize)
{
    File * fp = File::Create(pathname, File::OPEN|File::READ);
	if (!fp)
	{
		Logger::Error("Failed to open file: %s", pathname.GetAbsolutePathname().c_str());
		return 0;
	}
	fileSize = fp->GetSize();
	uint8 * bytes = new uint8[fileSize];
	uint32 dataRead = fp->Read(bytes, fileSize);
    
	if (dataRead != fileSize)
	{
		Logger::Error("Failed to read data from file: %s", pathname.GetAbsolutePathname().c_str());
		return 0;
	}

	SafeRelease(fp);
    return bytes;
};

void FileSystem::AttachArchive(const String & archiveName, const String & attachPath)
{
	ResourceArchive * resourceArchive = new ResourceArchive();

	if (!resourceArchive->Open(archiveName)) 
	{
		SafeRelease(resourceArchive);
		resourceArchive = NULL;
		return;
	}
	ResourceArchiveItem item;
	item.attachPath = attachPath;
	item.archive = resourceArchive;
	resourceArchiveList.push_back(item);
}


int32 FileSystem::Spawn(const String& command)
{
	int32 retCode = 0;
#if defined(__DAVAENGINE_MACOS__)
	retCode = std::system(command.c_str());
#elif defined(__DAVAENGINE_WIN32__) 

	/* std::system calls "start" command from Windows command line
	Start help:
	Starts a separate window to run a specified program or command.

	START ["title"] [/D path] [/I] [/MIN] [/MAX] [/SEPARATE | /SHARED]
	[/LOW | /NORMAL | /HIGH | /REALTIME | /ABOVENORMAL | /BELOWNORMAL]
	[/NODE <NUMA node>] [/AFFINITY <hex affinity mask>] [/WAIT] [/B]
	[command/program] [parameters]

	If we use "" for path to executable, start resolves it as title. So we need to specify call of start
	http://stackoverflow.com/questions/5681055/how-do-i-start-a-windows-program-with-spaces-in-the-path-from-perl

	*/

 	String startString = "start \"\" /WAIT " + command;
	retCode = std::system(startString.c_str());
#endif

	if(retCode != 0)

	{
		Logger::Warning("[FileSystem::Spawn] command (%s) has return code (%d)", command.c_str(), retCode);
	}

	return retCode;
}

#if defined(__DAVAENGINE_ANDROID__)

Set<String> FileSystem::dirSet;
Set<String> FileSystem::fileSet;

bool FileSystem::IsAPKPath(const String& path) const
{
	if (!path.empty() && path.c_str()[0] == '/')
		return false;
	return true;
}

void FileSystem::Init()
{
	YamlParser* parser = YamlParser::Create("~res:/fileSystem.yaml");
	if (parser)
	{
		const YamlNode* node = parser->GetRootNode();
		const YamlNode* dirList = node->Get("dirList");
		if (dirList)
		{
			const Vector<YamlNode*> vec = dirList->AsVector();
			for (uint32 i = 0; i < vec.size(); ++i)
				dirSet.insert(vec[i]->AsString());
		}
		const YamlNode* fileList = node->Get("fileList");
		if (fileList)
		{
			const Vector<YamlNode*> vec = fileList->AsVector();
			for (uint32 i = 0; i < vec.size(); ++i)
				fileSet.insert(vec[i]->AsString());
		}
	}
	SafeRelease(parser);
}
#endif

}




