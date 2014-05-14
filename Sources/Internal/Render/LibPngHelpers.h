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


#ifndef __DAVAENGINE_LIBPNG_HELPERS_H__
#define __DAVAENGINE_LIBPNG_HELPERS_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "Render/Image.h"
#include "FileSystem/FilePath.h"

namespace DAVA 
{

class Texture;
class Sprite;
class Image;

class LibPngWrapper
{
public:
    
    static bool IsPngFile(File *file);
    
	static int ReadPngFile(const FilePath & file, Image * image, PixelFormat targetFormat = FORMAT_INVALID);
	static int ReadPngFile(File *infile, Image * image, PixelFormat targetFormat = FORMAT_INVALID);
	static bool WritePngFile(const FilePath & fileName, int32 width, int32 height, uint8 * data, PixelFormat format);

    static uint32 GetDataSize(const FilePath &filePathname);

};

class PngImage : public BaseObject
{
protected:
	~PngImage();
public:
	PngImage();
	
	bool Create(int32 _width, int32 _height);
	bool CreateFromFBOSprite(Sprite * fboSprite);
	
	bool Load(const FilePath & filename);
	bool Save(const FilePath & filename);
	
	void DrawImage(int sx, int sy, PngImage * image);
	void DrawRect(const Rect2i & rect, uint32 color);

	uint8 * GetData() { return data; };
	int32 GetWidth() { return width; };
	int32 GetHeight() { return height; }; 
private:	
	int32		width;
	int32		height;
	uint8  *	data;
    PixelFormat format;
};
};

#endif // __PNG_IMAGE_H__
