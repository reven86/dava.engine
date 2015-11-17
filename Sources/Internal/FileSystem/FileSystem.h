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


#ifndef __DAVAENGINE_FILESYSTEM_H__
#define __DAVAENGINE_FILESYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "FileSystem/File.h"
#include "FileSystem/FilePath.h"

#if defined (__DAVAENGINE_ANDROID__)
#include "FileSystem/ZipFile.h"
#endif //__DAVAENGINE_ANDROID__
/**
	\defgroup filesystem File System
 */
namespace DAVA 
{
class ResourceArchive;
/**
	\ingroup filesystem
	\brief FileSystem is a wrapper class that allow to perform all basic filesystem operations
	
	Class is platform dependent but it must used in all places where you want to be sure that portability is an issue

	Supported platforms:
		Windows, MacOS X, iPhone OS
 
	\todo add functions to enumerate files in directories to be full functional FileSystem
	\todo refactoring of utils and ~res:/ ~doc:/ access for the project files
	\todo add support for pack files
*/
class FileSystem : public Singleton <FileSystem>
{
public:
	FileSystem();
	virtual ~FileSystem();
	
	/**
		\brief Function to delete file from filesystem
		\param[in] filePath full path for the file we want to delete
		\returns true if deletion was successful
	 */
	virtual bool DeleteFile(const FilePath & filePath);
	
	
	/*
		\brief Function to delete directory
		
		If isRecursive variable is false, function will succeed only in case if directory is empty.
	 
		\param[in] path full path to the directory you want to delete
		\param[in] isRecursive if true trying to delete all subfolders, if not just trying to delete this directory
		\returns true if this directory was deleted
	 */
	virtual bool DeleteDirectory(const FilePath & path, bool isRecursive = true);

	
	/*
		\brief Deletes all files in given directory
		if isRecursive is set function walks into all child directories and delete files there also.
		This funciton do not delete directoris, it delete only files
		\param[in] isRecursive if true go into child directories and delete files there also, false by default
		\returns number of deleted files
	*/ 
	virtual uint32 DeleteDirectoryFiles(const FilePath & path, bool isRecursive = false);

	enum eCreateDirectoryResult
	{
		DIRECTORY_CANT_CREATE = 0,
		DIRECTORY_EXISTS = 1,
		DIRECTORY_CREATED = 2,
	};
	/**
		\brief Function to create directory at filePath you've requested
		\param[in] filepath where you want to create a directory
		\returns true if directory created successfully
	 */
	virtual eCreateDirectoryResult CreateDirectory(const FilePath & filePath, bool isRecursive = false);
	
	/**
		\brief Function to retrieve current working directory
		\returns current working directory
	 */
	virtual const FilePath & GetCurrentWorkingDirectory();

	/**
		\brief Function to retrieve directory, which contain executable binary file
		\returns current directory, with  executable file
	 */
	virtual FilePath  GetCurrentExecutableDirectory();

	/**
		\brief Function to set current working directory
		\param[in] newWorkingDirectory new working directory to be set
		\returns true if directory set successfully
	 */
	virtual bool SetCurrentWorkingDirectory(const FilePath & newWorkingDirectory);
	
	/**
        \brief Function to retrieve current documents directory
        \returns current documents directory
     */
	virtual const FilePath & GetCurrentDocumentsDirectory();
    
    /**
         \brief Function to set current documents directory
         \param[in] newDocDirectory new documents directory to be set
     */
	virtual void SetCurrentDocumentsDirectory(const FilePath & newDocDirectory);
    
    /**
         \brief Function to set current documents directory to default
     */
	virtual void SetDefaultDocumentsDirectory();
    
    /**
         \brief Function to retrieve user's documents path
         \returns user's documents path
     */
	static const FilePath GetUserDocumentsPath();
    
    /**
         \brief Function to retrieve public documents path
         \returns public documents path
     */
	virtual const FilePath GetPublicDocumentsPath();

#if defined(__DAVAENGINE_APPLE__)  
    /**
        \brief Function to retrieve user’s home path
        \returns user’s home path
    */
    const FilePath GetHomePath();
#endif
    
	/**
		\brief Function check if specified path is a regular file
	*/
    bool IsFile(const FilePath& pathToCheck) const;

    /**
		\brief Function check if specified path is a directory
	 */
    bool IsDirectory(const FilePath& pathToCheck) const;

    /**
     \brief Function sets/removes exclusive lock to/from file.
     \param[in] filePath The name of the file to be locked/unlocked.
     \param[in] isLock true to lock file, false to unlock.
     \returns true if file was successfully locked/unlocked, false otherwise
	 */
    virtual bool LockFile(const FilePath & filePath, bool isLock);

    /**
     \brief Function checks whether the file is locked.
     \param[in] filePath The name of the file to be checked for lock.
     \returns true if file is locked, false if not locked
	 */
	virtual bool IsFileLocked(const FilePath & filePath) const;

	File *CreateFileForFrameworkPath(const FilePath & frameworkPath, uint32 attributes);

	/**
		\brief Copies an existing file to a new file.
		\param[in] existingFile The name of an existing file.
		\param[out] newFile The name of the new file.
		\returns true if file was successfully copied, false otherwise
	*/
	virtual bool CopyFile(const FilePath & existingFile, const FilePath & newFile, bool overwriteExisting = false);

	/**
		\brief Moves an existing file to a new file.
		\param[in] existingFile The name of an existing file.
		\param[out] newFile The name of the new file.
		\param[in] overwriteExisting signal to overwrite existing file with name newFile.
		\returns true if file was successfully moved, false otherwise
	*/
	virtual bool MoveFile(const FilePath & existingFile, const FilePath & newFile, bool overwriteExisting = false);

	/**
		\brief Copies directory to another existing directory.
		\param[in] sourceDirectory The name of an existing file.
		\param[out] destinationDirectory The name of the new file.
		\returns true if all files were successfully copied, false otherwise.
	*/
	virtual bool CopyDirectory(const FilePath & sourceDirectory, const FilePath & destinationDirectory, bool overwriteExisting = false);
    
    /**
        \brief Read whole file contents into new buffer. 
        If function returns zero error happened and it haven't loaded the file
        After you'll finish using the date you should DELETE returned buffer using function SafeDeleteArray.  
     
        \param[in] pathname path to the file we want to read
        \param[out] fileSize
        \returns pointer to newly created buffer with file contents
     */
    uint8 * ReadFileContents(const FilePath & pathname, uint32 & fileSize);
    
    
    /**
        \brief Read whole file contents into string.
        \param[in] pathname path to the file we want to read
        \returns string with whole file contents
     */
    String ReadFileContents(const FilePath & pathname);

	/**
		\brief Function to attach ResourceArchive to filesystem
	
		\param[in] archiveName pathname or local filename of archive we want to attach
		\param[in] attachPath path we attach our archive 
	*/ 
	virtual void AttachArchive(const String & archiveName, const String & attachPath);

	/**
	 \brief Invokes the command processor to execute a command
	 \param[in] command contains the system command to be executed
	 \returns platform-dependent
	 */
	int32 Spawn(const String& command);

	/**
	 \brief Marks folder as contains no media files to exclude it from index
	 */
	void MarkFolderAsNoMedia(const FilePath &folder);

    /**
    \brief Compares two files to check if theirs content is same. Ignores lineendings
    \param[in] filePath1 - path to one of files to compare
    \param[in] filePath2 - path to one of files to compare
    \param[in] ignoreEmptyLines - ignores any empty lines
    \returns true if files are equals and false if not
    */
    bool CompareTextFiles(const FilePath &filePath1, const FilePath &filePath2);

    /**
    \brief Compares two files to check if theirs content is same.
    \param[in] filePath1 - path to one of files to compare
    \param[in] filePath2 - path to one of files to compare
    \returns true if files are equals and false if not
    */
    bool CompareBinaryFiles(const FilePath &filePath1, const FilePath &filePath2);

    bool GetFileSize(const FilePath& path, uint32& size);

    /**
     \brief Function check if specified path exists on file system
     */
    bool Exists(const FilePath& filePath) const;

private:
    bool HasLineEnding(File *f);

	virtual eCreateDirectoryResult CreateExactDirectory(const FilePath & filePath);

	FilePath currentWorkingDirectory;
    FilePath currentDocDirectory;

	struct ResourceArchiveItem
	{
		ResourceArchive * archive;
		String attachPath;
	};

	List<ResourceArchiveItem> resourceArchiveList;
    Map<String, void*> lockedFileHandles;

	friend class File;
#if defined(__DAVAENGINE_ANDROID__)
	friend class ZipFile;
public:
	void Init();

private:
	bool IsAPKPath(const String& path) const;
	Set<String> fileSet;
	Set<String> dirSet;

#endif //#if defined(__DAVAENGINE_ANDROID__)
};
	
};

#endif // __DAVAENGINE_FILESYSTEM_H__
