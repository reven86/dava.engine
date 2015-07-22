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


#include "TextureConverter.h"
#include "DXTConverter.h"
#include "PVRConverter.h"
#include "Render/Texture.h"
#include "Render/TextureDescriptor.h"
#include "Render/GPUFamilyDescriptor.h"
#include "FileSystem/FileSystem.h"


ENUM_DECLARE(DAVA::TextureConverter::eConvertQuality)
{
    ENUM_ADD_DESCR(DAVA::TextureConverter::ECQ_FAST, "Developer Quality");
    ENUM_ADD_DESCR(DAVA::TextureConverter::ECQ_FASTEST, "Lower Quality");
    ENUM_ADD_DESCR(DAVA::TextureConverter::ECQ_NORMAL, "Normal Quality");
    ENUM_ADD_DESCR(DAVA::TextureConverter::ECQ_HIGH, "Better Quality");
    ENUM_ADD_DESCR(DAVA::TextureConverter::ECQ_VERY_HIGH, "Best Quality");
};

namespace DAVA
{
	FilePath TextureConverter::ConvertTexture(const TextureDescriptor &descriptor, eGPUFamily gpuFamily, bool updateAfterConversion, eConvertQuality quality)
	{
		const TextureDescriptor::Compression * compression = &descriptor.compression[gpuFamily];

		FilePath outputPath;
		auto compressedFormat = GPUFamilyDescriptor::GetCompressedFileFormat(gpuFamily, (DAVA::PixelFormat)compression->format);
		if(compressedFormat == IMAGE_FORMAT_PVR)
		{
            if (IMAGE_FORMAT_WEBP == descriptor.dataSettings.sourceFileFormat)
            {
                Logger::Error("Can not to convert from WebP (descriptor.pathname.GetAbsolutePathname().c_str()) to PVR");
                return FilePath();
            }

			Logger::FrameworkDebug("Starting PVR (%s) conversion (%s)...",
							   GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(compression->format), descriptor.pathname.GetAbsolutePathname().c_str());
			
            if(descriptor.dataSettings.GetIsNormalMap())
            {
                outputPath = PVRConverter::Instance()->ConvertNormalMapToPvr(descriptor, gpuFamily, quality);
            }
            else
            {
                outputPath = PVRConverter::Instance()->ConvertToPvr(descriptor, gpuFamily, quality);
            }
		}
		else if(compressedFormat == IMAGE_FORMAT_DDS)
		{
			DAVA::Logger::FrameworkDebug("Starting DXT(%s) conversion (%s)...",
							   GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(compression->format), descriptor.pathname.GetAbsolutePathname().c_str());
			
			
			if(descriptor.IsCubeMap())
			{
				outputPath = DXTConverter::ConvertCubemapToDxt(descriptor, gpuFamily);
			}
			else
			{
				outputPath = DXTConverter::ConvertToDxt(descriptor, gpuFamily);
			}
		}
		else
		{
			DVASSERT(false);
		}

		if(updateAfterConversion)
		{
			bool wasUpdated = descriptor.UpdateCrcForFormat(gpuFamily);
			if(wasUpdated)
			{
				// Potential problem may occur in case of multithread convertion of
				// one texture: Save() will dump to drive unvalid compression info
				// and final variant of descriptor must be dumped again after finishing
				// of all threads.
				descriptor.Save();
			}
		}
		
		return outputPath;
	}
	
	FilePath TextureConverter::GetOutputPath(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
	{
		return descriptor.CreatePathnameForGPU(gpuFamily);
	}
};