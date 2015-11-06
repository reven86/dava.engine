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


#ifndef __DAVAENGINE_DEFINITION_FILE_H__
#define __DAVAENGINE_DEFINITION_FILE_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "Math/Math2D.h"

namespace DAVA
{
 
class DefinitionFile 
{
public:
    bool Load(const FilePath& filename);
    bool LoadPNGDef(const FilePath& filename, const FilePath& pathToProcess);

    DefinitionFile();
	~DefinitionFile();
	
	void ClearPackedFrames();
	void LoadPNG(const FilePath & fullname, const FilePath & processDirectoryPath);

	Size2i GetFrameSize(int frame) const;
	int GetFrameWidth(int frame) const;
	int GetFrameHeight(int frame) const;

	FilePath    filename;
	int			frameCount;
	int			spriteWidth;
	int			spriteHeight;
	Rect2i		* frameRects;

	Vector<String> pathsInfo;
    Vector<String> frameNames;
};

};


#endif // __DAVAENGINE_DEFINITION_FILE_H__