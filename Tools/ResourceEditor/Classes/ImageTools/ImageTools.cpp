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


#include "ImageTools/ImageTools.h"
#include "Render/LibPngHelpers.h"
#include "Render/LibPVRHelper.h"
#include "Render/LibDxtHelper.h"

#include "TextureCompression/TextureConverter.h"

#include "Render/GPUFamilyDescriptor.h"

#include "Main/QtUtils.h"

using namespace DAVA;

uint32 ImageTools::GetTexturePhysicalSize(const TextureDescriptor *descriptor, const eGPUFamily forGPU)
{
	uint32 size = 0;
	
	Vector<FilePath> files;
	
	if(descriptor->IsCubeMap() &&
	   GPU_UNKNOWN == forGPU)
	{
		Vector<FilePath> faceNames;
		Texture::GenerateCubeFaceNames(descriptor->pathname.GetAbsolutePathname().c_str(), faceNames);
        
        files.reserve(faceNames.size());
		for(size_t i = 0 ; i < faceNames.size(); ++i)
		{
			files.push_back(FilePath(faceNames[i]));
		}
	}
	else
	{
		FilePath imagePathname = GPUFamilyDescriptor::CreatePathnameForGPU(descriptor, forGPU);
		files.push_back(imagePathname);
	}
	
	for(size_t i = 0; i < files.size(); ++i)
	{
		//FilePath imagePathname = GPUFamilyDescriptor::CreatePathnameForGPU(descriptor, forGPU);
		const FilePath& imagePathname = files[i];
		
		File *imageFile = File::Create(imagePathname, File::OPEN | File::READ);
		if(!imageFile)
		{
			Logger::Error("[ImageTools::GetTexturePhysicalSize] Can't open file %s", imagePathname.GetAbsolutePathname().c_str());
			return 0;
		}
		
		
		if(ImageLoader::IsPNGFile(imageFile))
		{
			size += LibPngWrapper::GetDataSize(imagePathname);
		}
		else if(ImageLoader::IsDDSFile(imageFile))
		{
			size += LibDxtHelper::GetDataSize(imagePathname);
		}
		else if(ImageLoader::IsPVRFile(imageFile))
		{
			size += LibPVRHelper::GetDataSize(imagePathname);
		}
		else
		{
			DVASSERT(false);
		}
		
		SafeRelease(imageFile);
	}
	
    return size;
}


void ImageTools::ConvertImage(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily forGPU, const DAVA::PixelFormat format)
{
	if(!descriptor || (format == FORMAT_INVALID)) return;

	TextureConverter::CleanupOldTextures(descriptor, forGPU, format);
	TextureConverter::ConvertTexture(*descriptor, forGPU, true);
}
