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


#ifndef __DAVAENGINE_CUBEMAP_H__
#define __DAVAENGINE_CUBEMAP_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{

    
class Cubemap
{
public:
    
	//VI: each face is optional
	enum CubemapFace
	{
		CUBE_FACE_POSITIVE_X = 0,
		CUBE_FACE_NEGATIVE_X = 1,
		CUBE_FACE_POSITIVE_Y = 2,
		CUBE_FACE_NEGATIVE_Y = 3,
		CUBE_FACE_POSITIVE_Z = 4,
		CUBE_FACE_NEGATIVE_Z = 5,
		CUBE_FACE_MAX_COUNT = 6,
		CUBE_FACE_INVALID = 0xFFFFFFFF
	};
	
	static void GenerateCubeFaceNames(const FilePath & baseName, Vector<FilePath>& faceNames);
	static void GenerateCubeFaceNames(const FilePath & baseName, const Vector<String>& faceNameSuffixes, Vector<FilePath>& faceNames);

public:
    
    static int32 CUBE_FACE_MAPPING[CUBE_FACE_MAX_COUNT];
    static DAVA::String FACE_NAME_SUFFIX[CUBE_FACE_MAX_COUNT];

};
    

};
#endif // __DAVAENGINE_CUBEMAP_H__
