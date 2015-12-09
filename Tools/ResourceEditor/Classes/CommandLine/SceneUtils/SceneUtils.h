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


#ifndef __SCENE_UTILS_H__
#define __SCENE_UTILS_H__

#include "DAVAEngine.h"

class SceneUtils
{
public:

	SceneUtils();
    ~SceneUtils();
    
    void CleanFolder(const DAVA::FilePath &folderPathname, DAVA::Set<DAVA::String> &errorLog);
    
    void SetInFolder(const DAVA::FilePath &folderPathname);
    void SetOutFolder(const DAVA::FilePath &folderPathname);
    
    bool CopyFile(const DAVA::FilePath &filePathname, DAVA::Set<DAVA::String> &errorLog);
    void PrepareFolderForCopyFile(const DAVA::String &filename, DAVA::Set<DAVA::String> &errorLog);


    DAVA::FilePath GetNewFilePath(const DAVA::FilePath &oldPathname) const;
    
    void AddFile(const DAVA::FilePath &sourcePath);
    void CopyFiles(DAVA::Set<DAVA::String> &errorLog);

private:
    void PrepareDestination(DAVA::Set<DAVA::String> &errorLog);

public:

    DAVA::FilePath dataFolder;
    DAVA::FilePath dataSourceFolder;
    DAVA::String workingFolder;
    
    DAVA::Map<DAVA::FilePath, DAVA::FilePath> filesForCopy;
};

namespace RenderObjectsFlusher
{
DAVA_DEPRECATED(void Flush());
}

#endif // __SCENE_UTILS_H__