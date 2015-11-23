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


#ifndef __DAVAENGINE_PNGIMAGEEXT_H__
#define __DAVAENGINE_PNGIMAGEEXT_H__

#include "Base/BaseTypes.h"
#include "Render/Image/Image.h"
#include "TexturePacker/TextureAtlas.h"

namespace DAVA
{

class PngImageExt
{
public:
    PngImageExt();
    ~PngImageExt();

    bool Create(uint32 width, uint32 height);

    bool Read(const FilePath &filename);
    void Write(const FilePath &filename, ImageQuality quality = DEFAULT_IMAGE_QUALITY);

    void DrawImage(const ImageCell& drawRect, const Rect2i& imageOffsetRect, PngImageExt* image);
    void DrawImage(int32 sx, int32 sy, PngImageExt *image, const Rect2i &srcRect);

    void DrawRect(const Rect2i &rect, uint32 color);

    void FindNonOpaqueRect(Rect2i &rect);

    bool ConvertToFormat(PixelFormat format);

    void DitherAlpha();

    inline uint32 GetWidth() const;
    inline uint32 GetHeight() const;

private:

    inline uint8 * GetData() const;

    bool IsHorzLineOpaque(int32 y);
    bool IsVertLineOpaque(int32 x);

    Color GetDitheredColorForPoint(int32 x, int32 y);

    ScopedPtr<Image> internalData;
};

inline uint8 * PngImageExt::GetData() const
{
    DVASSERT(internalData);
    return internalData->GetData();
}


inline uint32 PngImageExt::GetWidth() const
{
    DVASSERT(internalData);
    return internalData->GetWidth();
}

inline uint32 PngImageExt::GetHeight() const
{
    DVASSERT(internalData);
    return internalData->GetHeight();
}

};


#endif // __DAVAENGINE_PNGIMAGEEXT_H__
