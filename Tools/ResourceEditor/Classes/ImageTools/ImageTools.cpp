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

#include "TextureCompression/TextureConverter.h"

#include "Render/GPUFamilyDescriptor.h"
#include "Render/PixelFormatDescriptor.h"

#include "Main/QtUtils.h"

using namespace DAVA;

uint32 ImageTools::GetTexturePhysicalSize(const TextureDescriptor *descriptor, const eGPUFamily forGPU, uint32 baseMipMaps)
{
	uint32 size = 0;
	
	Vector<FilePath> files;
	
	if(descriptor->IsCubeMap() && forGPU == GPU_PNG)
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
        ImageInfo info = ImageSystem::Instance()->GetImageInfo(imagePathname);
        if (!info.isEmpty())
        {
            const auto formatSizeBits = PixelFormatDescriptor::GetPixelFormatSizeInBits(info.format);
            
            auto m = Min(baseMipMaps, info.mipmapsCount - 1);
            for( ; m < info.mipmapsCount; ++m)
            {
                const auto w = (info.width >> m);
                const auto h = (info.height >> m);
                
                size += (w * h * formatSizeBits / 8);
            }
        }
        else
        {
            Logger::Error("[ImageTools::GetTexturePhysicalSize] Can't detect type of file %s", imagePathname.GetStringValue().c_str());
        }
    }
	
    return size;
}


void ImageTools::ConvertImage(const DAVA::TextureDescriptor *descriptor, const DAVA::eGPUFamily forGPU, const DAVA::PixelFormat format, DAVA::TextureConverter::eConvertQuality quality)
{
	if(!descriptor || (format == FORMAT_INVALID)) return;

	TextureConverter::CleanupOldTextures(descriptor, forGPU, format);
	TextureConverter::ConvertTexture(*descriptor, forGPU, true, quality);
}

bool ImageTools::SplitImage(const FilePath &pathname, Set<String> &errorLog)
{
    Image *loadedImage = CreateTopLevelImage(pathname);
    if(!loadedImage)
    {
        errorLog.insert(String(Format("Can't load image %s", pathname.GetAbsolutePathname().c_str())));
        return false;
    }
    
    if(loadedImage->GetPixelFormat() != FORMAT_RGBA8888)
    {
        errorLog.insert(String(Format("Incorrect image format %s. Must be RGBA8888", PixelFormatDescriptor::GetPixelFormatString(loadedImage->GetPixelFormat()))));
        return false;
    }
    
    Channels channels = CreateSplittedImages(loadedImage);
    
    FilePath folder(pathname.GetDirectory());
    
    SaveImage(channels.red, folder + "r.png");
    SaveImage(channels.green, folder + "g.png");
    SaveImage(channels.blue, folder + "b.png");
    SaveImage(channels.alpha, folder + "a.png");
    
    channels.ReleaseImages();
    SafeRelease(loadedImage);
    return true;
}

bool ImageTools::MergeImages(const FilePath &folder, Set<String> &errorLog)
{
    DVASSERT(folder.IsDirectoryPathname());
    
    Channels channels(LoadImage(folder + "r.png"), LoadImage(folder + "g.png"), LoadImage(folder + "b.png"), LoadImage(folder + "a.png"));
    
    if(channels.IsEmpty())
    {
        errorLog.insert(String(Format("Can't load one or more channel images from folder %s", folder.GetAbsolutePathname().c_str())));
        channels.ReleaseImages();
        return false;
    }
    
    if(!channels.HasFormat(FORMAT_A8))
    {
        errorLog.insert(String("Can't merge images. Source format must be Grayscale 8bit"));
        channels.ReleaseImages();
        return false;
    }
    
    if(!channels.ChannelesResolutionEqual())
    {
        errorLog.insert(String("Can't merge images. Source images must have same size"));
        channels.ReleaseImages();
        return false;
    }
    
    Image *mergedImage = CreateMergedImage(channels);
    
    ImageSystem::Instance()->Save(folder + "merged.png", mergedImage);
    channels.ReleaseImages();
    SafeRelease(mergedImage);
    return true;
}

void ImageTools::SaveImage(Image *image, const FilePath &pathname)
{
    ImageSystem::Instance()->Save(pathname, image);
}

Image * ImageTools::LoadImage(const FilePath &pathname)
{
    return CreateTopLevelImage(pathname);
}

Channels ImageTools::CreateSplittedImages(DAVA::Image* originalImage)
{
    DAVA::Image* r = Image::Create(originalImage->width, originalImage->height, FORMAT_A8);
    DAVA::Image* g = Image::Create(originalImage->width, originalImage->height, FORMAT_A8);
    DAVA::Image* b = Image::Create(originalImage->width, originalImage->height, FORMAT_A8);
    DAVA::Image* a = Image::Create(originalImage->width, originalImage->height, FORMAT_A8);
    
    int32 size = originalImage->width * originalImage->height;
    int32 pixelSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(FORMAT_RGBA8888);
    for(int32 i = 0; i < size; ++i)
    {
        int32 offset = i * pixelSize;
        r->data[i] = originalImage->data[offset];
        g->data[i] = originalImage->data[offset + 1];
        b->data[i] = originalImage->data[offset + 2];
        a->data[i] = originalImage->data[offset + 3];
    }
    return Channels(r,g,b,a);
}

DAVA::Image* ImageTools::CreateMergedImage(const Channels& channels)
{
    if(!channels.ChannelesResolutionEqual() || !channels.HasFormat(FORMAT_A8))
    {
        return NULL;
    }
    Image *mergedImage = Image::Create(channels.red->width, channels.red->height, FORMAT_RGBA8888);
    int32 size = mergedImage->width * mergedImage->height;
    int32 pixelSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(FORMAT_RGBA8888);
    for(int32 i = 0; i < size; ++i)
    {
        int32 offset = i * pixelSize;
        mergedImage->data[offset] = channels.red->data[i];
        mergedImage->data[offset + 1] = channels.green->data[i];
        mergedImage->data[offset + 2] = channels.blue->data[i];
        mergedImage->data[offset + 3] = channels.alpha->data[i];
    }
    return mergedImage;
}

void ImageTools::SetChannel(DAVA::Image* image, eComponentsRGBA channel, DAVA::uint8 value)
{
    if(image->format != FORMAT_RGBA8888)
    {
        return;
    }
    int32 size = image->width * image->height;
    int32 pixelSize = PixelFormatDescriptor::GetPixelFormatSizeInBytes(FORMAT_RGBA8888);
    int32 offset = channel;
    for( int32 i = 0; i < size; ++i, offset += pixelSize)
    {
        image->data[offset] = value;
    }
}

void Channels::ReleaseImages()
{
    DAVA::SafeRelease(red);
    DAVA::SafeRelease(green);
    DAVA::SafeRelease(blue);
    DAVA::SafeRelease(alpha);
}
