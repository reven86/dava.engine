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



#ifndef __DAVAENGINE_DXT_HELPER_H__
#define __DAVAENGINE_DXT_HELPER_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"

namespace DAVA 
{

class Image;
class File;

class LibDxtHelper
{
public:
	
	static bool IsDxtFile(const FilePath & fileName);
	static bool IsDxtFile(File * file);

	//input data only in RGBA8888
	static bool WriteDdsFile(const FilePath & fileName, int32 width, int32 height, uint8 ** data, uint32 dataCount, PixelFormat compressionFormat, bool generateMipmaps);

	static bool ReadDxtFile(const FilePath & fileName, Vector<Image*> &imageSet, bool forceSoftwareConvertation = false);
	static bool ReadDxtFile(File * file, Vector<Image*> &imageSet, bool forceSoftwareConvertation = false);

	static bool DecompressImageToRGBA(const DAVA::Image & image, Vector<DAVA::Image*> &imageSet, bool forceSoftwareConvertation = false);

	static PixelFormat GetPixelFormat(const FilePath & fileName);
	static PixelFormat GetPixelFormat(File * file);
	
	static bool GetTextureSize(const FilePath & fileName, uint32 & width, uint32 & height);
	static bool GetTextureSize(File * file, uint32 & width, uint32 & height);

	static uint32 GetMipMapLevelsCount(const FilePath & fileName);
	static uint32 GetMipMapLevelsCount(File * file);

	static uint32 GetDataSize(const FilePath & fileName);
	static uint32 GetDataSize(File * file);

	static bool AddCRCIntoMetaData(const FilePath &filePathname);
	static uint32 GetCRCFromFile(const FilePath &filePathname);
	
private:

	static bool GetCRCFromDDSHeader(const FilePath &filePathname, uint32* tag, uint32* outputCRC);
	
	//input data only in RGBA8888
	static bool WriteDxtFile(const FilePath & fileName, int32 width, int32 height, uint8 ** data, uint32 dataCount, PixelFormat compressionFormat, bool generateMipmaps);
	static bool WriteAtcFile(const FilePath & fileName, int32 width, int32 height, uint8 ** data, uint32 dataCount, PixelFormat compressionFormat, bool generateMipmaps);
};

};

#endif // __DAVAENGINE_DXT_HELPER_H__