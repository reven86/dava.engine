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


#ifndef __DAVAENGINE_FILE_PATH_H__
#define __DAVAENGINE_FILE_PATH_H__

#include "Base/BaseTypes.h"
//#include "Base/Introspection.h"

namespace DAVA
{

static const char* localResourcesPath = "/mnt/sdcard/DavaProject/";
    
/**
	\ingroup filesystem
	\brief class to work with file pathname
 */
class FilePath
{
public:
    
    enum ePathType
    {
		PATH_EMPTY = -1,			// empty path, newly created with empty string
        PATH_IN_FILESYSTEM = 0,     // not framework path /Users/... or c:/...
        PATH_IN_RESOURCES,          // ~res:/...
        PATH_IN_DOCUMENTS,          // ~doc:/...
        PATH_IN_MEMORY              // FBO, TEXT, memory file
    };
    
    
public:

	FilePath();
    FilePath(const FilePath & path);
    FilePath(FilePath&& path) DAVA_NOEXCEPT;
    FilePath(const String & sourcePath);
    FilePath(const char * sourcePath);
    FilePath(const FilePath & directory, const String & filename);
    FilePath(const String & directory, const String & filename);
    FilePath(const char *directory, const String & filename);

	~FilePath();

    /**
        \brief Function to retrieve FilePath with new extension without changing of source FilePath object
        \param[in] pathname is source FilePath object
        \param[in] extension is new extension
        \returns resolved FilePath object with new extension
	 */
    static FilePath CreateWithNewExtension(const FilePath &pathname, const String &extension);


    FilePath& operator=(const FilePath & path);
    FilePath& operator=(FilePath&& path) DAVA_NOEXCEPT;
    FilePath operator+(const String & path) const;
    FilePath& operator+=(const String & path);

    bool operator==(const FilePath & path) const;
	bool operator!=(const FilePath & path) const;
    
	/*
        \brief Function to check is filepath empty or no
        \returns true if absolutePathname is not empty
	 */
    inline bool IsEmpty() const;

	/*
        \brief Function to check is filepath represent folder path
        \returns true if absolutePathname has '/' as last character
	 */
    bool IsDirectoryPathname() const;

	/**
        \brief Function to retrieve pathname
        \returns pathname value
	 */
    String GetAbsolutePathname() const;

    /**
        \brief Function to retrieve filename from pathname. Filename for path "/Users/Folder/image.png" is "image.png".
        \returns filename value
	 */
    String GetFilename() const;

    /**
        \brief Function to retrieve basename from pathname. Basename for path "/Users/Folder/image.png" is "image".
        \returns basename value
	 */
    String GetBasename() const;
    
	/**
        \brief Function to retrieve extension from pathname. Extension for path "/Users/Folder/image.png" is ".png".
        \returns extension value
	 */
    String GetExtension() const;

	/**
        \brief Function to retrieve directory from pathname. Directory for path "/Users/Folder/image.png" is "/Users/Folder/".
        \returns directory value
	 */
    FilePath GetDirectory() const;
    
    
	/**
        \brief Function to retrieve relative pathname for current working directory
        \returns relative path value
	 */
    String GetRelativePathname() const;

	/**
        \brief Function to retrieve relative pathname for exact directory
        \param[in] forDirectory is exact directory for relative path calculation
        \returns relative path value
	 */
    String GetRelativePathname(const FilePath &forDirectory) const;
	String GetRelativePathname(const String &forDirectory) const;
    String GetRelativePathname(const char * forDirectory) const;
    
    /**
        \brief Function to retrieve string path value, passed in constructor
        \returns relative string path value
	 */
    const String & GetStringValue() const;
    
    /**
        \brief Function to retrieve string path value as URL for Web Browser
        \returns path as URL
     */
    String AsURL() const;

    /**
        \brief Function for replacement of original filename
        \param[in] filename is new filename
	 */
    void ReplaceFilename(const String &filename);

	/**
        \brief Function for replacement of original basename
        \param[in] basename is new basename
	 */
    void ReplaceBasename(const String &basename);

	/**
        \brief Function for replacement of original extension
        \param[in] extension is new extension
	 */
    void ReplaceExtension(const String &extension);
    
	/**
        \brief Function for replacement of original directory
        \param[in] directory is new directory
	 */
    void ReplaceDirectory(const String &directory);
    void ReplaceDirectory(const FilePath &directory);
    
	/**
        \brief Function to modify absolute to be path to folder. For example "Users/Document" after function call will be "Users/Document/"
	 */
    FilePath & MakeDirectoryPathname();

	/**
        \brief Function to truncate extension from path
	 */
    void TruncateExtension();
    
    /**
        \brief Function to retrieve last directory name from FilePath that represents directory pathname
        \returns last directory name
	 */
    String GetLastDirectoryName() const;
    
    
	/**
        \brief Function for comparison with extension of filepath object
        \param[in] extension is extension for comparison
	 */
	bool IsEqualToExtension(const String & extension) const;


	/**
        \brief Function to retrieve path in framework style: with ~res: or ~doc:
        \param[in] type of FilePath representation
		\returns pathname value for requested type
	 */
	String GetFrameworkPath() const;

	/**
        \brief Function to set system path bundle path to project path for resolving pathnames such as "~res:/Gfx/image.png"
	 */
    static void InitializeBundleName();

	/**
	 \brief Temporary function to set system path for NPAPI plugins for resolving pathnames such as "~res:/Gfx/image.png"
	 */
	#if defined (__DAVAENGINE_NPAPI__)
	static void InitializeBundleNameNPAPI(const String& pathToNPAPIPlugin);
	#endif // #if defined (__DAVAENGINE_NPAPI__)
	
	/**
        \brief Function to set project path for resolving pathnames such as "~res:/Gfx/image.png"
        \param[in] new project path
	 */
	static void SetBundleName(const FilePath &newBundlePath);
		
	/**
        \brief Function to retrieve project path for resolving pathnames such as "~res:/Gfx/image.png"
        \returns project path 
	 */
    static const FilePath & GetBundleName();

    /**
        \brief Function to retrieve full path relative current documents folder
        \returns path relative corrent documents folder
     */
    static FilePath FilepathInDocuments(const char * relativePathname);
    
    /**
        \brief Function to retrieve full path relative current documents folder
        \returns path relative corrent documents folder
     */
    static FilePath FilepathInDocuments(const String & relativePathname);

    
    /**
        \brief Function to retrieve type of path
        \returns type of path
     */
    inline ePathType GetType() const;
    
	static bool ContainPath(const FilePath& basePath, const FilePath& partPath);
	static bool ContainPath(const FilePath& basePath, const String & partPath);
	static bool ContainPath(const FilePath& basePath, const char * partPath);

	static void AddResourcesFolder(const FilePath & folder);
	static void AddTopResourcesFolder(const FilePath & folder);
    static void RemoveResourcesFolder(const FilePath & folder);
    static const List<FilePath>& GetResourcesFolders();

    bool Exists() const;
    
	int32 Compare(const FilePath &right) const;

protected:
    
    void Initialize(const String &pathname);

    String ResolveResourcesPath() const;
    

    static String NormalizePathname(const String &pathname);
    
    static String MakeDirectory(const String &pathname);

    static String AbsoluteToRelative(const FilePath &directoryPathname, const FilePath &absolutePathname);

    static String GetFilename(const String &pathname);

    static String GetSystemPathname(const String &pathname, const ePathType pType);
	String GetFrameworkPathForPrefix(const String &typePrefix, const ePathType pType) const;
    
    static bool IsAbsolutePathname(const String &pathname);

    static ePathType GetPathType(const String &pathname);
    
    static bool IsGlobbing(const String &pathname);
    
public:
    static String AddPath(const FilePath &folder, const String & addition);

protected:
    
    String absolutePathname;
    ePathType pathType;

    static List<FilePath> resourceFolders;
};

bool operator < (const FilePath& left, const FilePath& right);

    
inline bool FilePath::IsEmpty() const
{
    return (pathType == PATH_EMPTY);
}
    
inline FilePath::ePathType FilePath::GetType() const
{
    return pathType;
}
  
};

#endif //__DAVAENGINE_FILE_PATH_H__
