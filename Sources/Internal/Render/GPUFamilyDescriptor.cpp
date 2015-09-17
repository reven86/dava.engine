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


#include "Render/GPUFamilyDescriptor.h"
#include "FileSystem/Logger.h"
#include "Debug/DVAssert.h"
#include "FileSystem/FilePath.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"
#include "Render/TextureDescriptor.h"
#include "Render/Texture.h"
#include "Render/PixelFormatDescriptor.h"
#include "Render/Image/ImageSystem.h"

namespace DAVA
{
    
GPUFamilyDescriptor::GPUData GPUFamilyDescriptor::gpuData[GPU_FAMILY_COUNT];

void GPUFamilyDescriptor::SetupGPUParameters()
{
    SetupGPUFormats();
    SetupGPUPostfixes();
}

void GPUFamilyDescriptor::SetupGPUFormats()
{
    //pvr ios
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_RGBA8888] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_RGBA5551] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_RGBA4444] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_RGB888] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_RGB565] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_A8] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_PVR4] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_PVR2] = IMAGE_FORMAT_PVR;

    //es30
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_PVR2_2] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_PVR4_2] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_EAC_R11_UNSIGNED] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_EAC_R11_SIGNED] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_EAC_RG11_SIGNED] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_EAC_RG11_UNSIGNED] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_ETC2_RGB] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_ETC2_RGBA] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_IOS].availableFormats[FORMAT_ETC2_RGB_A1] = IMAGE_FORMAT_PVR;


    //pvr android
    gpuData[GPU_POWERVR_ANDROID].availableFormats[FORMAT_RGBA8888] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_ANDROID].availableFormats[FORMAT_RGBA5551] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_ANDROID].availableFormats[FORMAT_RGBA4444] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_ANDROID].availableFormats[FORMAT_RGB888] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_ANDROID].availableFormats[FORMAT_RGB565] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_ANDROID].availableFormats[FORMAT_A8] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_ANDROID].availableFormats[FORMAT_PVR4] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_ANDROID].availableFormats[FORMAT_PVR2] = IMAGE_FORMAT_PVR;
    gpuData[GPU_POWERVR_ANDROID].availableFormats[FORMAT_ETC1] = IMAGE_FORMAT_PVR;

    //tegra
    gpuData[GPU_TEGRA].availableFormats[FORMAT_RGBA8888] = IMAGE_FORMAT_PVR;
    gpuData[GPU_TEGRA].availableFormats[FORMAT_RGBA5551] = IMAGE_FORMAT_PVR;
    gpuData[GPU_TEGRA].availableFormats[FORMAT_RGBA4444] = IMAGE_FORMAT_PVR;
    gpuData[GPU_TEGRA].availableFormats[FORMAT_RGB888] = IMAGE_FORMAT_PVR;
    gpuData[GPU_TEGRA].availableFormats[FORMAT_RGB565] = IMAGE_FORMAT_PVR;
    gpuData[GPU_TEGRA].availableFormats[FORMAT_A8] = IMAGE_FORMAT_PVR;
    gpuData[GPU_TEGRA].availableFormats[FORMAT_DXT1] = IMAGE_FORMAT_DDS;
    gpuData[GPU_TEGRA].availableFormats[FORMAT_DXT1A] = IMAGE_FORMAT_DDS;
    gpuData[GPU_TEGRA].availableFormats[FORMAT_DXT3] = IMAGE_FORMAT_DDS;
    gpuData[GPU_TEGRA].availableFormats[FORMAT_DXT5] = IMAGE_FORMAT_DDS;
    gpuData[GPU_TEGRA].availableFormats[FORMAT_DXT5NM] = IMAGE_FORMAT_DDS;
    gpuData[GPU_TEGRA].availableFormats[FORMAT_ETC1] = IMAGE_FORMAT_PVR;

    //mali
    gpuData[GPU_MALI].availableFormats[FORMAT_RGBA8888] = IMAGE_FORMAT_PVR;
    gpuData[GPU_MALI].availableFormats[FORMAT_RGBA5551] = IMAGE_FORMAT_PVR;
    gpuData[GPU_MALI].availableFormats[FORMAT_RGBA4444] = IMAGE_FORMAT_PVR;
    gpuData[GPU_MALI].availableFormats[FORMAT_RGB888] = IMAGE_FORMAT_PVR;
    gpuData[GPU_MALI].availableFormats[FORMAT_RGB565] = IMAGE_FORMAT_PVR;
    gpuData[GPU_MALI].availableFormats[FORMAT_A8] = IMAGE_FORMAT_PVR;
    gpuData[GPU_MALI].availableFormats[FORMAT_ETC1] = IMAGE_FORMAT_PVR;
    
    //adreno
    gpuData[GPU_ADRENO].availableFormats[FORMAT_RGBA8888] = IMAGE_FORMAT_PVR;
    gpuData[GPU_ADRENO].availableFormats[FORMAT_RGBA5551] = IMAGE_FORMAT_PVR;
    gpuData[GPU_ADRENO].availableFormats[FORMAT_RGBA4444] = IMAGE_FORMAT_PVR;
    gpuData[GPU_ADRENO].availableFormats[FORMAT_RGB888] = IMAGE_FORMAT_PVR;
    gpuData[GPU_ADRENO].availableFormats[FORMAT_RGB565] = IMAGE_FORMAT_PVR;
    gpuData[GPU_ADRENO].availableFormats[FORMAT_A8] = IMAGE_FORMAT_PVR;
    gpuData[GPU_ADRENO].availableFormats[FORMAT_ETC1] = IMAGE_FORMAT_PVR;
    gpuData[GPU_ADRENO].availableFormats[FORMAT_ATC_RGB] = IMAGE_FORMAT_DDS;
	gpuData[GPU_ADRENO].availableFormats[FORMAT_ATC_RGBA_EXPLICIT_ALPHA] = IMAGE_FORMAT_DDS;
	gpuData[GPU_ADRENO].availableFormats[FORMAT_ATC_RGBA_INTERPOLATED_ALPHA] = IMAGE_FORMAT_DDS;

    //DX11
    gpuData[GPU_DX11].availableFormats[FORMAT_RGBA8888] = IMAGE_FORMAT_PVR;
    gpuData[GPU_DX11].availableFormats[FORMAT_RGBA5551] = IMAGE_FORMAT_PVR;
    gpuData[GPU_DX11].availableFormats[FORMAT_RGBA4444] = IMAGE_FORMAT_PVR;
    gpuData[GPU_DX11].availableFormats[FORMAT_RGB888] = IMAGE_FORMAT_PVR;
    gpuData[GPU_DX11].availableFormats[FORMAT_RGB565] = IMAGE_FORMAT_PVR;
    gpuData[GPU_DX11].availableFormats[FORMAT_A8] = IMAGE_FORMAT_PVR;
    gpuData[GPU_DX11].availableFormats[FORMAT_DXT1] = IMAGE_FORMAT_DDS;
    gpuData[GPU_DX11].availableFormats[FORMAT_DXT1A] = IMAGE_FORMAT_DDS;
    gpuData[GPU_DX11].availableFormats[FORMAT_DXT3] = IMAGE_FORMAT_DDS;
    gpuData[GPU_DX11].availableFormats[FORMAT_DXT5] = IMAGE_FORMAT_DDS;
    gpuData[GPU_DX11].availableFormats[FORMAT_DXT5NM] = IMAGE_FORMAT_DDS;

    //ORIGIN
    gpuData[GPU_ORIGIN].availableFormats[FORMAT_RGBA8888] = IMAGE_FORMAT_UNKNOWN;
    gpuData[GPU_ORIGIN].availableFormats[FORMAT_RGB888] = IMAGE_FORMAT_UNKNOWN;
    gpuData[GPU_ORIGIN].availableFormats[FORMAT_RGBA5551] = IMAGE_FORMAT_UNKNOWN;
    gpuData[GPU_ORIGIN].availableFormats[FORMAT_A8] = IMAGE_FORMAT_UNKNOWN;
    gpuData[GPU_ORIGIN].availableFormats[FORMAT_A16] = IMAGE_FORMAT_UNKNOWN;
}

void GPUFamilyDescriptor::SetupGPUPostfixes()
{
    gpuData[GPU_POWERVR_IOS].name = "PowerVR_iOS";
    gpuData[GPU_POWERVR_IOS].prefix = ".PowerVR_iOS";
    gpuData[GPU_POWERVR_ANDROID].name = "PowerVR_Android";
    gpuData[GPU_POWERVR_ANDROID].prefix = ".PowerVR_Android";
    gpuData[GPU_TEGRA].name = "tegra";
    gpuData[GPU_TEGRA].prefix = ".tegra";
    gpuData[GPU_MALI].name = "mali";
    gpuData[GPU_MALI].prefix = ".mali";
    gpuData[GPU_ADRENO].name = "adreno";
    gpuData[GPU_ADRENO].prefix = ".adreno";
    gpuData[GPU_DX11].name = "dx11";
    gpuData[GPU_DX11].prefix = ".dx11";
    gpuData[GPU_ORIGIN].name = "origin";
    gpuData[GPU_ORIGIN].prefix = "";
}

const Map<PixelFormat, ImageFormat> & GPUFamilyDescriptor::GetAvailableFormatsForGpu(eGPUFamily gpuFamily)
{
    DVASSERT(0 <= gpuFamily && gpuFamily < GPU_FAMILY_COUNT);
    
    return gpuData[gpuFamily].availableFormats;
}

eGPUFamily GPUFamilyDescriptor::GetGPUForPathname(const FilePath &pathname)
{
    const String filename = pathname.GetFilename();
    
    for(int32 i = 0; i < GPU_DEVICE_COUNT; ++i)
    {
        if (String::npos != filename.rfind(gpuData[i].prefix))
        {
            return static_cast<eGPUFamily>(i);
        }
    }

    const String ext = pathname.GetExtension();
    bool isUncompressed = TextureDescriptor::IsSourceTextureExtension(ext);
    return isUncompressed ? GPU_ORIGIN : GPU_INVALID;
}

const String & GPUFamilyDescriptor::GetGPUName(const eGPUFamily gpuFamily)
{
    DVASSERT(0 <= gpuFamily && gpuFamily < GPU_FAMILY_COUNT);

    return gpuData[gpuFamily].name;
}

const String& GPUFamilyDescriptor::GetGPUPrefix(const eGPUFamily gpuFamily)
{
    DVASSERT(0 <= gpuFamily && gpuFamily < GPU_FAMILY_COUNT);

    return gpuData[gpuFamily].prefix;
}

eGPUFamily GPUFamilyDescriptor::GetGPUByName(const String & name)
{
    for(int32 i = 0; i < GPU_FAMILY_COUNT; ++i)
    {
        eGPUFamily gpu = (eGPUFamily)i;
        if(name == gpuData[gpu].name)
        {
            return gpu;
        }
    }
    
    return GPU_INVALID;
}

bool GPUFamilyDescriptor::IsFormatSupported(const eGPUFamily gpu, const PixelFormat format)
{
    if (gpu < 0 || gpu >= GPU_FAMILY_COUNT || format == FORMAT_INVALID)
    {
        return false;
    }
    return gpuData[gpu].availableFormats.find(format) != gpuData[gpu].availableFormats.end();
}

ImageFormat GPUFamilyDescriptor::GetCompressedFileFormat(const eGPUFamily gpuFamily, const PixelFormat pixelFormat)
{
    if (!IsGPUForDevice(gpuFamily) || pixelFormat == FORMAT_INVALID)
        return IMAGE_FORMAT_UNKNOWN;

    auto& gpuFormats = gpuData[gpuFamily].availableFormats;
    auto formatFound = gpuFormats.find(pixelFormat);
    if (formatFound == gpuFormats.end())
    {
        Logger::Error("[GPUFamilyDescriptor::GetFileFormat: can't find pixel format %s for gpu %s]", PixelFormatDescriptor::GetPixelFormatString(pixelFormat), gpuData[gpuFamily].name.c_str());
        return IMAGE_FORMAT_UNKNOWN;
    }

    return formatFound->second;
}

eGPUFamily GPUFamilyDescriptor::ConvertValueToGPU(const int32 value)
{
    if (value >= 0 && value < GPU_FAMILY_COUNT)
    {
        return static_cast<eGPUFamily>(value);
    }
    else if (value == -1) // -1 is an old value, left for compatibility: some old tex files may contain gpu = -1
    {
        return GPU_ORIGIN;
    }
    else
    {
        return GPU_INVALID;
    }
}

bool GPUFamilyDescriptor::IsGPUForDevice(const eGPUFamily gpu)
{
    return (gpu >= 0 && gpu < GPU_DEVICE_COUNT);
}

};
