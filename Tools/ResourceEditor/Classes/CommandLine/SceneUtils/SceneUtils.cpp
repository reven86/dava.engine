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


#include "SceneUtils.h"

using namespace DAVA;

SceneUtils::SceneUtils()
{
}

SceneUtils::~SceneUtils()
{
}

void SceneUtils::CleanFolder(const FilePath &folderPathname, Set<String> &errorLog)
{
    bool ret = FileSystem::Instance()->DeleteDirectory(folderPathname);
    if(!ret)
    {
        bool folderExists = FileSystem::Instance()->IsDirectory(folderPathname);
        if(folderExists)
        {
            errorLog.insert(String(Format("[CleanFolder] ret = %d, folder = %s", ret, folderPathname.GetAbsolutePathname().c_str())));
        }
    }
}

void SceneUtils::SetInFolder(const FilePath &folderPathname)
{
    DVASSERT(folderPathname.IsDirectoryPathname());
    dataSourceFolder = folderPathname;
}

void SceneUtils::SetOutFolder(const FilePath &folderPathname)
{
	DVASSERT(folderPathname.IsDirectoryPathname());
    dataFolder = folderPathname;
}


bool SceneUtils::CopyFile(const FilePath &filePathname, Set<String> &errorLog)
{
	String workingPathname = filePathname.GetRelativePathname(dataSourceFolder);

	PrepareFolderForCopyFile(workingPathname, errorLog);
    
    bool retCopy = FileSystem::Instance()->CopyFile(dataSourceFolder + workingPathname, dataFolder + workingPathname);
    if(!retCopy)
    {
        errorLog.insert(String(Format("Can't copy %s from %s to %s",
                                      workingPathname.c_str(),
                                      dataSourceFolder.GetAbsolutePathname().c_str(),
                                      dataFolder.GetAbsolutePathname().c_str())));
    }
    
    return retCopy;
}

void SceneUtils::PrepareFolderForCopyFile(const String &filename, Set<String> &errorLog)
{
    FilePath newFolderPath = (dataFolder + filename).GetDirectory();
    
    if(!FileSystem::Instance()->IsDirectory(newFolderPath))
    {
        FileSystem::eCreateDirectoryResult retCreate = FileSystem::Instance()->CreateDirectory(newFolderPath, true);
        if(FileSystem::DIRECTORY_CANT_CREATE == retCreate)
        {
            errorLog.insert(String(Format("Can't create folder %s", newFolderPath.GetAbsolutePathname().c_str())));
        }
    }
    
    FileSystem::Instance()->DeleteFile(dataFolder + filename);
}

DAVA::FilePath SceneUtils::GetNewFilePath(const DAVA::FilePath &oldPathname) const
{
	String workingPathname = oldPathname.GetRelativePathname(dataSourceFolder);
    return dataFolder + workingPathname;
}

void SceneUtils::AddFile(const DAVA::FilePath &sourcePath)
{
    String workingPathname = sourcePath.GetRelativePathname(dataSourceFolder);
    FilePath destinationPath = dataFolder + workingPathname;

    if(sourcePath != destinationPath)
    {
        DVASSERT(!sourcePath.IsEmpty());
        DVASSERT(!destinationPath.IsEmpty());

        filesForCopy[sourcePath] = destinationPath;
    }
}

void SceneUtils::CopyFiles(Set<String> &errorLog)
{
    PrepareDestination(errorLog);
    
    auto endIt = filesForCopy.end();
    for(auto it = filesForCopy.begin(); it != endIt; ++it)
    {
		bool retCopy = false;

        if (FileSystem::Instance()->Exists(it->first))
        {
			FileSystem::Instance()->DeleteFile(it->second);
			retCopy = FileSystem::Instance()->CopyFile(it->first, it->second);
		}

		if(!retCopy)
		{
			errorLog.insert(String(Format("Can't copy %s to %s",
				it->first.GetAbsolutePathname().c_str(),
				it->second.GetAbsolutePathname().c_str())));
		}
	}
}

void SceneUtils::PrepareDestination(DAVA::Set<DAVA::String> &errorLog)
{
    DAVA::Set<DAVA::FilePath> folders;

    DAVA::Map<DAVA::FilePath, DAVA::FilePath>::const_iterator endMapIt = filesForCopy.end();
    for(DAVA::Map<DAVA::FilePath, DAVA::FilePath>::const_iterator it = filesForCopy.begin(); it != endMapIt; ++it)
    {
        folders.insert(it->second.GetDirectory());
    }
    
    DAVA::Set<DAVA::FilePath>::const_iterator endSetIt = folders.end();
    for(DAVA::Set<DAVA::FilePath>::const_iterator it = folders.begin(); it != endSetIt; ++it)
    {
        if (!FileSystem::Instance()->Exists(*it))
        {
            FileSystem::eCreateDirectoryResult retCreate = FileSystem::Instance()->CreateDirectory((*it), true);
            if(FileSystem::DIRECTORY_CANT_CREATE == retCreate)
            {
                errorLog.insert(String(Format("Can't create folder %s", (*it).GetAbsolutePathname().c_str())));
            }
        }
    }
}

/*
 * RenderObjectsFlusher implementation
 * temporary (hopefully!) solution to clean-up RHI's objects
 * when there is no run/render loop in the application
 */

DAVA_DEPRECATED(void RenderObjectsFlusher::Flush())
{
    static const rhi::HTexture nullTexture;
    static const rhi::Viewport nullViewport(0, 0, 1, 1);

    auto currentFrame = rhi::GetCurrentFrameSyncObject();
    while (!rhi::SyncObjectSignaled(currentFrame))
    {
        Renderer::BeginFrame();
        RenderHelper::CreateClearPass(nullTexture, 0, DAVA::Color::Clear, nullViewport);
        Renderer::EndFrame();
    }
}
