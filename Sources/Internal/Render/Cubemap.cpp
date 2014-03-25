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


#include "Render/Cubemap.h"
#include "Render/GPUFamilyDescriptor.h"

namespace DAVA
{
    
int32 Cubemap::CUBE_FACE_MAPPING[CUBE_FACE_MAX_COUNT] =
{
	Cubemap::CUBE_FACE_POSITIVE_X,
	Cubemap::CUBE_FACE_NEGATIVE_X,
	Cubemap::CUBE_FACE_POSITIVE_Y,
	Cubemap::CUBE_FACE_NEGATIVE_Y,
	Cubemap::CUBE_FACE_POSITIVE_Z,
	Cubemap::CUBE_FACE_NEGATIVE_Z
};

DAVA::String Cubemap::FACE_NAME_SUFFIX[CUBE_FACE_MAX_COUNT] =
{
    DAVA::String("_px"),
    DAVA::String("_nx"),
    DAVA::String("_py"),
    DAVA::String("_ny"),
    DAVA::String("_pz"),
    DAVA::String("_nz")
};
	

void Cubemap::GenerateCubeFaceNames(const FilePath & baseName, Vector<FilePath>& faceNames)
{
	static Vector<String> defaultSuffixes;
	if(defaultSuffixes.empty())
	{
		for(int i = 0; i < CUBE_FACE_MAX_COUNT; ++i)
		{
			defaultSuffixes.push_back(FACE_NAME_SUFFIX[i]);
		}
	}
	
	GenerateCubeFaceNames(baseName, defaultSuffixes, faceNames);
}

void Cubemap::GenerateCubeFaceNames(const FilePath & filePath, const Vector<String>& faceNameSuffixes, Vector<FilePath>& faceNames)
{
	faceNames.clear();
	
	String basename = filePath.GetBasename();
		
	for(size_t i = 0; i < faceNameSuffixes.size(); ++i)
	{
		DAVA::FilePath faceFilePath = filePath;
		faceFilePath.ReplaceFilename(basename + faceNameSuffixes[i] + GPUFamilyDescriptor::GetFilenamePostfix(GPU_UNKNOWN, FORMAT_INVALID));
			
		faceNames.push_back(faceFilePath);
	}
}

};
