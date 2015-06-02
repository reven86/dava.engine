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


#include "Render/PixelFormatDescriptor.h"
#include "Utils/Utils.h"
#include "Render/Renderer.h"

#if defined (__DAVAENGINE_OPENGL__)


#ifdef __DAVAENGINE_ANDROID__
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif //GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif //GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#endif //__DAVAENGINE_ANDROID__

namespace DAVA 
{

#if !defined(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG)
	#define  GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG    0
#endif
#if !defined(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG)
	#define  GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG    0
#endif

#if !defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
	#define  GL_COMPRESSED_RGB_S3TC_DXT1_EXT    0
#endif

#if !defined(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	#define  GL_COMPRESSED_RGBA_S3TC_DXT1_EXT    0
#endif

#if !defined(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
	#define  GL_COMPRESSED_RGBA_S3TC_DXT3_EXT    0
#endif

#if !defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
	#define  GL_COMPRESSED_RGB_S3TC_DXT1_EXT    0
#endif

#if !defined(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
	#define  GL_COMPRESSED_RGBA_S3TC_DXT5_EXT    0
#endif

#if !defined(GL_ETC1_RGB8_OES)
	#define  GL_ETC1_RGB8_OES    0
#endif

#if !defined(GL_ATC_RGB_AMD)
	#define  GL_ATC_RGB_AMD    0
#endif

#if !defined(GL_ATC_RGBA_EXPLICIT_ALPHA_AMD)
	#define  GL_ATC_RGBA_EXPLICIT_ALPHA_AMD    0
#endif

#if !defined(GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD)
	#define  GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD    0
#endif

#if !defined(GL_COMPRESSED_R11_EAC)
	#define  GL_COMPRESSED_R11_EAC    0
#endif

#if !defined(GL_COMPRESSED_SIGNED_R11_EAC)
	#define  GL_COMPRESSED_SIGNED_R11_EAC    0
#endif

#if !defined(GL_COMPRESSED_RG11_EAC)
	#define  GL_COMPRESSED_RG11_EAC    0
#endif

#if !defined(GL_COMPRESSED_SIGNED_RG11_EAC)
	#define  GL_COMPRESSED_SIGNED_RG11_EAC    0
#endif

#if !defined(GL_COMPRESSED_RGB8_ETC2)
	#define  GL_COMPRESSED_RGB8_ETC2    0
#endif

#if !defined(GL_COMPRESSED_RGBA8_ETC2_EAC)
	#define  GL_COMPRESSED_RGBA8_ETC2_EAC    0
#endif

#if !defined(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2)
	#define  GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2    0
#endif

#if !defined(GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG)
	#define  GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG    0
#endif

#if !defined(GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG)
	#define  GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG    0
#endif


PixelFormatDescriptor PixelFormatDescriptor::pixelDescriptors[FORMAT_COUNT];

PixelFormatDescriptor::PixelFormatDescriptor()
    : format(rhi::TextureFormat(-1))
	,   formatID(FORMAT_INVALID)
	,   pixelSize(0)
	,   isHardwareSupported(false)
{
}



void PixelFormatDescriptor::InitializePixelFormatDescriptors()
{

    const rhi::TextureFormat UNSUPPORTED_FORMAT = (rhi::TextureFormat) -1;
	const RenderCaps& caps = Renderer::GetCaps();    

    DVASSERT(FORMAT_COUNT == 33); // add new format below

	//SetPixelDescription(FORMAT_INVALID, FastName("WRONG FORMAT"), 0);
	SetPixelDescription(FORMAT_RGBA8888, FastName("RGBA8888"), 32, rhi::TEXTURE_FORMAT_R8G8B8A8, true);
	SetPixelDescription(FORMAT_RGBA5551, FastName("RGBA5551"), 16, rhi::TEXTURE_FORMAT_R5G5B5A1, true);
	SetPixelDescription(FORMAT_RGBA4444, FastName("RGBA4444"), 16, rhi::TEXTURE_FORMAT_R4G4B4A4, true);
    SetPixelDescription(FORMAT_RGB888, FastName("RGB888"), 24, rhi::TEXTURE_FORMAT_R8G8B8, true);
    SetPixelDescription(FORMAT_RGB565, FastName("RGB565"), 16, rhi::TEXTURE_FORMAT_R5G6B5, true);
    SetPixelDescription(FORMAT_A8, FastName("A8"), 8, rhi::TEXTURE_FORMAT_R8, true);

#if defined (__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_IPHONE__)
    SetPixelDescription(FORMAT_A16, FastName("A16"), 16, rhi::TEXTURE_FORMAT_R16, false);
#else 
    SetPixelDescription(FORMAT_A16, FastName("A16"), 16, rhi::TEXTURE_FORMAT_R16, true);
#endif
    
    SetPixelDescription(FORMAT_RGBA16161616, FastName("RGBA16161616"), 64, rhi::TEXTURE_FORMAT_A16R16G16B16, caps.isFloat16Supported);
    
    
    SetPixelDescription(FORMAT_RGBA32323232, FastName("RGBA32323232"), 128, rhi::TEXTURE_FORMAT_A32R32G32B32, caps.isFloat32Supported);

    SetPixelDescription(FORMAT_PVR4, FastName("PVR4"), 4, UNSUPPORTED_FORMAT, caps.isPVRTCSupported);
    SetPixelDescription(FORMAT_PVR2, FastName("PVR2"), 2, UNSUPPORTED_FORMAT, caps.isPVRTCSupported);

    SetPixelDescription(FORMAT_DXT1, FastName("DXT1"), 4, rhi::TEXTURE_FORMAT_DXT1, caps.isDXTSupported);
    SetPixelDescription(FORMAT_DXT1A, FastName("DXT1a"), 4, UNSUPPORTED_FORMAT, caps.isDXTSupported);
    SetPixelDescription(FORMAT_DXT3, FastName("DXT3"), 8, rhi::TEXTURE_FORMAT_DXT3, caps.isDXTSupported);
    SetPixelDescription(FORMAT_DXT5, FastName("DXT5"), 8, rhi::TEXTURE_FORMAT_DXT5, caps.isDXTSupported);
    SetPixelDescription(FORMAT_DXT5NM, FastName("DXT5nm"), 8, UNSUPPORTED_FORMAT, caps.isDXTSupported);

    SetPixelDescription(FORMAT_ETC1, FastName("ETC1"), 8, rhi::TEXTURE_FORMAT_ETC1, caps.isETCSupported);

    SetPixelDescription(FORMAT_ATC_RGB, FastName("ATC_RGB"), 4, rhi::TEXTURE_FORMAT_ATC_RGB, caps.isATCSupported);
    SetPixelDescription(FORMAT_ATC_RGBA_EXPLICIT_ALPHA, FastName("ATC_RGBA_EXPLICIT_ALPHA"), 8, rhi::TEXTURE_FORMAT_ATC_RGBA_EXPLICIT, caps.isATCSupported);
    SetPixelDescription(FORMAT_ATC_RGBA_INTERPOLATED_ALPHA, FastName("ATC_RGBA_INTERPOLATED_ALPHA"), 8, rhi::TEXTURE_FORMAT_ATC_RGBA_INTERPOLATED, caps.isATCSupported);

    SetPixelDescription(FORMAT_PVR2_2, FastName("PVR2_2"), 2, rhi::TEXTURE_FORMAT_PVRTC2_2BPP_RGBA, caps.isPVRTC2Supported);
    SetPixelDescription(FORMAT_PVR4_2, FastName("PVR4_2"), 4, rhi::TEXTURE_FORMAT_PVRTC2_4BPP_RGBA, caps.isPVRTC2Supported);

    SetPixelDescription(FORMAT_EAC_R11_UNSIGNED, FastName("EAC_R11"), 8, rhi::TEXTURE_FORMAT_EAC_R11_UNSIGNED, caps.isOpenGLES3Supported);
    SetPixelDescription(FORMAT_EAC_R11_SIGNED, FastName("EAC_R11_SIGNED"), 8, rhi::TEXTURE_FORMAT_EAC_R11_SIGNED, caps.isOpenGLES3Supported);
    SetPixelDescription(FORMAT_EAC_RG11_UNSIGNED, FastName("EAC_RG11"), 8, rhi::TEXTURE_FORMAT_EAC_R11G11_UNSIGNED, caps.isOpenGLES3Supported);
    SetPixelDescription(FORMAT_EAC_RG11_SIGNED, FastName("EAC_RG11_SIGNED"), 8, rhi::TEXTURE_FORMAT_EAC_R11G11_SIGNED, caps.isOpenGLES3Supported);

    SetPixelDescription(FORMAT_ETC2_RGB, FastName("ETC2_RGB"), 4, rhi::TEXTURE_FORMAT_ETC2_R8G8B8, caps.isOpenGLES3Supported);
    SetPixelDescription(FORMAT_ETC2_RGBA, FastName("ETC2_RGBA"), 4, rhi::TEXTURE_FORMAT_ETC2_R8G8B8A8, caps.isOpenGLES3Supported);
    SetPixelDescription(FORMAT_ETC2_RGB_A1, FastName("ETC2_RGB_A1"), 4, rhi::TEXTURE_FORMAT_ETC2_R8G8B8A1, caps.isOpenGLES3Supported);

#if defined (__DAVAENGINE_WIN32__)
    SetPixelDescription(FORMAT_BGR888, FastName("BGR888"), 24, UNSUPPORTED_FORMAT, false);
#endif

}

void PixelFormatDescriptor::SetPixelDescription(const PixelFormat formatID, const FastName &name, uint8 size, rhi::TextureFormat format, bool hardwareSupported)
{
    DVASSERT((0 <= formatID) && (formatID < FORMAT_COUNT));
    
    pixelDescriptors[formatID].formatID = formatID;
    pixelDescriptors[formatID].name = name;
    pixelDescriptors[formatID].pixelSize = size;
    pixelDescriptors[formatID].format = format;    
	pixelDescriptors[formatID].isHardwareSupported = hardwareSupported;	
}

const PixelFormatDescriptor & PixelFormatDescriptor::GetPixelFormatDescriptor(const PixelFormat formatID)
{
    DVASSERT((0 <= formatID) && (formatID < FORMAT_COUNT));
    return pixelDescriptors[formatID];
}

int32 PixelFormatDescriptor::GetPixelFormatSizeInBits(const PixelFormat formatID)
{
	DVASSERT((0 <= formatID) && (formatID < FORMAT_COUNT));
    return pixelDescriptors[formatID].pixelSize;
}

int32 PixelFormatDescriptor::GetPixelFormatSizeInBytes(const PixelFormat formatID)
{
    int32 bits = GetPixelFormatSizeInBits(formatID);
    if(bits < 8)
    {   // To detect wrong situations
        Logger::Warning("[Texture::GetPixelFormatSizeInBytes] format takes less than byte");
    }
    
    return  bits / 8;
}

const char * PixelFormatDescriptor::GetPixelFormatString(const PixelFormat formatID)
{
    DVASSERT((0 <= formatID) && (formatID < FORMAT_COUNT));
    return pixelDescriptors[formatID].name.c_str();
}

PixelFormat PixelFormatDescriptor::GetPixelFormatByName(const FastName &formatName)
{
    for(int32 i = 0; i < FORMAT_COUNT; ++i)
    {
        if(formatName == pixelDescriptors[i].name)
        {
            return pixelDescriptors[i].formatID;
        }
    }
    
    return FORMAT_INVALID;
}
 
};

#endif //#if defined (__DAVAENGINE_OPENGL__)
