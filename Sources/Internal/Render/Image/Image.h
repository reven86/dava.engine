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


#ifndef __DAVAENGINE_IMAGE_H__
#define __DAVAENGINE_IMAGE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Utils/Utils.h"

#include "MemoryManager/MemoryProfiler.h"

namespace DAVA 
{

class File;
#ifdef __DAVAENGINE_IPHONE__

class SaveToSystemPhotoCallbackReceiver
{
public:
    virtual void SaveToSystemPhotosFinished() = 0;
};
    
#endif

    

class Image : public BaseObject
{
    DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(ALLOC_POOL_IMAGE)

protected:
	virtual ~Image();
public:
	Image();
	
	static Image * Create(uint32 width, uint32 height, PixelFormat format);
	static Image * CreateFromData(uint32 width, uint32 height, PixelFormat format, const uint8 *data);
    
    static Image * CreatePinkPlaceholder(bool checkers = true);
    void MakePink(bool checkers = true);

	inline uint32 GetWidth() const;
	inline uint32 GetHeight() const;
	inline uint8 * GetData() const;
	inline PixelFormat GetPixelFormat() const;

#ifdef __DAVAENGINE_IPHONE__
    void *GetUIImage();
#endif

    bool Save(const FilePath &path) const;

#ifdef __DAVAENGINE_IPHONE__
    void SaveToSystemPhotos(SaveToSystemPhotoCallbackReceiver* callback = 0);
#endif

    Vector<Image *> CreateMipMapsImages(bool isNormalMap = false);

    bool Normalize();

    // changes size of image canvas to required size, if new size is bigger, sets 0 to all new pixels
    void ResizeCanvas(uint32 newWidth, uint32 newHeight);
    
	// changes size of image to required size (without any filtration)
	void ResizeImage(uint32 newWidth, uint32 newHeight);
    
	static Image* CopyImageRegion(const Image* imageToCopy,
								  uint32 newWidth, uint32 newHeight,
								  uint32 xOffset = 0, uint32 yOffset = 0);
	static Image* CopyImageRegion(const Image* imageToCopy, const Rect& rect);

	void InsertImage(const Image* image, uint32 dstX, uint32 dstY,
					 uint32 srcX = 0, uint32 srcY = 0,
					 uint32 srcWidth = -1, uint32 srcHeight = -1);
	void InsertImage(const Image* image, const Vector2& dstPos,
					 const Rect& srcRect);

    // changes size of image canvas to square
    void ResizeToSquare();
    void FlipVertical();
    void FlipHorizontal();
    void RotateDeg(int degree);
    void Rotate90Right();
    void Rotate90Left();

private:

	template<class Type>
	void FlipVertical(Type *buffer, uint32 width, uint32 height);

	template<class Type>
	void FlipHorizontal(Type *buffer, uint32 width, uint32 height);

    template<class Type>
    void Rotate90Right(Type *srcBuffer, Type *dstBuffer, uint32 sideLen);

    template<class Type>
    void Rotate90Left(Type *srcBuffer, Type *dstBuffer, uint32 sideLen);

public:
    uint32 dataSize;
	uint32	width:16;
	uint32	height:16;

	uint8 * data;

    uint32 mipmapLevel;
    PixelFormat format:8;
	
	uint32 cubeFaceID;
    
};


template<class Type>
void Image::FlipVertical(Type *buffer, uint32 width, uint32 height)
{
	const uint32 halfHeight = (height / 2 );
	for(uint32 x = 0; x < width; ++x)
	{
		for(uint32 y = 0; y < halfHeight; ++y )
		{
			Swap(buffer[y * width + x], buffer[(height - 1 - y) * width + x]);
		}
	}
}

template<class Type>
void Image::FlipHorizontal(Type *buffer, uint32 width, uint32 height)
{
	const uint32 halfWidth = width / 2;
	const uint32 maxY = height * width;

	for(uint32 y = 0; y < maxY; y += width)
	{
		for(uint32 x = 0; x < halfWidth; ++x)
		{
			Swap(buffer[y + x], buffer[y + width - x - 1]);
		}
	}
}

template<class Type>
void Image::Rotate90Right(Type *src, Type *dst, uint32 sideLen)
{
    for (uint32 y = 0; y < sideLen; ++y)
    {
        for (uint32 x = 0; x < sideLen; ++x)
        {
            dst[(sideLen - y - 1) + (x * sideLen)] = src[x + y * sideLen];
        }
    }
}

template<class Type>
void Image::Rotate90Left(Type *src, Type *dst, uint32 sideLen)
{
    for (uint32 y = 0; y < sideLen; ++y)
    {
        for (uint32 x = 0; x < sideLen; ++x)
        {
            dst[x + y * sideLen] = src[(sideLen - y - 1) + (x * sideLen)];
        }
    }
}


// Implementation of inline functions
uint32 Image::GetWidth() const
{
	return width;
}
uint32 Image::GetHeight() const
{
	return height;
}
uint8 * Image::GetData() const
{
	return data;
}
PixelFormat Image::GetPixelFormat() const
{
	return format;
}



	
};

#endif // __DAVAENGINE_IMAGE_H__
