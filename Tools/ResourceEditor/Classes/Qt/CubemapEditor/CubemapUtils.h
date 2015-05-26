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


#ifndef __CUBEMAP_UTILS_H__
#define __CUBEMAP_UTILS_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

#define CUBEMAPEDITOR_FACE_PX 0
#define CUBEMAPEDITOR_FACE_NX 1
#define CUBEMAPEDITOR_FACE_PY 2
#define CUBEMAPEDITOR_FACE_NY 3
#define CUBEMAPEDITOR_FACE_PZ 4
#define CUBEMAPEDITOR_FACE_NZ 5

class CubemapUtils
{
public:
	
	static void GenerateFaceNames(const DAVA::String& baseName, DAVA::Vector<DAVA::String>& faceNames);
	
	static int GetMaxFaces();
	static int MapUIToFrameworkFace(int uiFace);
	static int MapFrameworkToUIFace(int frameworkFace);
	static const DAVA::String& GetFaceNameSuffix(int faceId);
	static const DAVA::String& GetDefaultFaceExtension();
	static DAVA::FilePath GetDialogSavedPath(const DAVA::String& key, const DAVA::String& defaultValue);
};

#endif /* defined(__CUBEMAP_UTILS_H__) */
