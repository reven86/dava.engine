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


#ifndef __IMAGE_TOOLS_H__
#define __IMAGE_TOOLS_H__

#include "TextureCompression/TextureConverter.h"
#include "DAVAEngine.h"

struct Channels
{
    DAVA::Image* red;
    DAVA::Image* green;
    DAVA::Image* blue;
    DAVA::Image* alpha;
    
    Channels(DAVA::Image* _red = NULL, DAVA::Image* _green = NULL, DAVA::Image* _blue = NULL, DAVA::Image* _alpha = NULL):
    red(_red),
    green(_green),
    blue(_blue),
    alpha(_alpha)
    {}
    
    inline bool IsEmpty() const
    {
        return (!red || !green || !blue || !alpha);
    }
    
    inline bool HasFormat(DAVA::PixelFormat format) const
    {
        return (red->GetPixelFormat()   == format &&
                green->GetPixelFormat() == format &&
                blue->GetPixelFormat()  == format &&
                alpha->GetPixelFormat() == format);
    }
    
    inline bool ChannelesResolutionEqual() const
    {
        return (red->width == green->width && red->width == blue->width && red->width == alpha->width) &&
               (red->height == green->height && red->height == blue->height && red->height == alpha->height);
    }
    
    void ReleaseImages();
};

class ImageTools
{
public:
    
     enum eComponentsRGBA
     {
         COLOR_RED		= 0,
         COLOR_GREEN,
         COLOR_BLUE,
         COLOR_ALPHA,
     };
    
    static DAVA::uint32 GetTexturePhysicalSize(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily forGPU, DAVA::uint32 baseMipMaps = 0);
	static void ConvertImage(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily forGPU, const DAVA::PixelFormat format, DAVA::TextureConverter::eConvertQuality quality);
    
    static bool SplitImage(const DAVA::FilePath &pathname, DAVA::Set<DAVA::String> &errorLog);
    
    static bool MergeImages(const DAVA::FilePath &folder, DAVA::Set<DAVA::String> &errorLog);
    
    static Channels CreateSplittedImages(DAVA::Image* originalImage);
    
    static DAVA::Image* CreateMergedImage(const Channels& channes);
    
    static void SetChannel(DAVA::Image* image, eComponentsRGBA channel,  DAVA::uint8 value);

private:
    
    static void SaveImage(DAVA::Image *image, const DAVA::FilePath &pathname);
 
    static DAVA::Image * LoadImage(const DAVA::FilePath &pathname);
};

#endif // __IMAGE_TOOLS_H__
