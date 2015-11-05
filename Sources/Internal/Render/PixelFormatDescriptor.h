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


#ifndef __DAVAENGINE_PIXEL_FORMAT_DESCRIPTOR_H__
#define __DAVAENGINE_PIXEL_FORMAT_DESCRIPTOR_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Render/RHI/rhi_Type.h"

namespace DAVA
{
/**
	\ingroup render
	\brief Class that represents pixel format for internal using in our SDK. 
 */

#if defined (__DAVAENGINE_OPENGL__)


class PixelFormatDescriptor
{
public:
    static void SetHardwareSupportedFormats();

    static int32 GetPixelFormatSizeInBytes(const PixelFormat formatID);
    static int32 GetPixelFormatSizeInBits(const PixelFormat formatID);

    static const char * GetPixelFormatString(const PixelFormat format);
    static PixelFormat GetPixelFormatByName(const FastName &formatName);

    static const PixelFormatDescriptor& GetPixelFormatDescriptor(const PixelFormat formatID);

private:
    static UnorderedMap<PixelFormat, PixelFormatDescriptor, std::hash<uint8>> pixelDescriptors;

public:
    static rhi::TextureFormat TEXTURE_FORMAT_INVALID;

    PixelFormat formatID;
    FastName name;
    uint8 pixelSize;
    rhi::TextureFormat format;
    bool isHardwareSupported;
};

#endif //#if defined (__DAVAENGINE_OPENGL__)

};
#endif // __DAVAENGINE_PIXEL_FORMAT_DESCRIPTOR_H__
