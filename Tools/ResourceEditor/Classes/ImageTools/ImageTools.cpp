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


void Channels::ReleaseImages()
{
    DAVA::SafeRelease(red);
    DAVA::SafeRelease(green);
    DAVA::SafeRelease(blue);
    DAVA::SafeRelease(alpha);
}


uint32 ImageTools::GetTexturePhysicalSize(const TextureDescriptor *descriptor, const eGPUFamily forGPU)
{
	uint32 size = 0;
	
	Vector<FilePath> files;
	
	if(descriptor->IsCubeMap() && forGPU == GPU_ORIGIN)
	{
		Vector<FilePath> faceNames;
		descriptor->GetFacePathnames(faceNames);
        
        files.reserve(faceNames.size());
		for(auto faceName : faceNames)
		{
            if (!faceName.IsEmpty())
			    files.push_back(faceName);
		}
	}
	else
	{
		FilePath imagePathname = descriptor->CreatePathnameForGPU(forGPU);
		files.push_back(imagePathname);
	}
	
	for(size_t i = 0; i < files.size(); ++i)
	{
		const FilePath& imagePathname = files[i];
        ImageInfo info = ImageSystem::Instance()->GetImageInfo(imagePathname);
        if (!info.isEmpty())
        {
            size += info.dataSize;
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
        return nullptr;
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


QImage ImageTools::FromDavaImage(const DAVA::FilePath & pathname)
{
    auto image = LoadImage(pathname);
    if(image)
    {
        QImage img = FromDavaImage(image);
        SafeRelease(image);
        
        return img;
    }
    
    return QImage();
}

QImage ImageTools::FromDavaImage(DAVA::Image *image)
{
    QImage qtImage;
    
    if(nullptr != image)
    {
        QRgb *line = nullptr;
        
        switch(image->format)
        {
            case DAVA::FORMAT_DXT1:
            case DAVA::FORMAT_DXT1A:
            case DAVA::FORMAT_DXT3:
            case DAVA::FORMAT_DXT5:
            case DAVA::FORMAT_DXT5NM:
            {
                Vector<Image* > vec;
                LibDdsHelper::DecompressImageToRGBA(*image, vec, true);
                if(vec.size() == 1)
                {
                    qtImage = FromDavaImage(vec.front());
                }
                else
                {
                    DAVA::Logger::Error("Error during conversion from DDS to QImage.");
                }
                
                for_each(vec.begin(), vec.end(), SafeRelease<DAVA::Image>);
                
                break;
            }
            case DAVA::FORMAT_PVR4:
            case DAVA::FORMAT_PVR2:
            case DAVA::FORMAT_RGBA8888:
            {
                DAVA::uint32 *data = (DAVA::uint32 *) image->data;
                DAVA::uint32 c;
                
                qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);
                
                // convert DAVA:RGBA8888 into Qt ARGB8888
                for (int y = 0; y < (int)image->height; y++)
                {
                    line = (QRgb *) qtImage.scanLine(y);
                    for (int x = 0; x < (int)image->width; x++)
                    {
                        c = data[y * image->width + x];
                        line[x] = (c & 0xFF00FF00) | ((c & 0x00FF0000) >> 16) | ((c & 0x000000FF) << 16);
                    }
                }
            }
                break;
                
            case DAVA::FORMAT_RGBA5551:
            {
                DAVA::uint16 *data = (DAVA::uint16 *) image->data;
                DAVA::uint32 c;
                
                qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);
                
                // convert DAVA:RGBA5551 into Qt ARGB8888
                for (int y = 0; y < (int)image->height; y++)
                {
                    line = (QRgb *) qtImage.scanLine(y);
                    for (int x = 0; x < (int)image->width; x++)
                    {
                        DAVA::uint32 a;
                        DAVA::uint32 r;
                        DAVA::uint32 g;
                        DAVA::uint32 b;
                        
                        c = data[y * image->width + x];
                        r = (c >> 11) & 0x1f;
                        g = (c >> 6) & 0x1f;
                        b = (c >> 1) & 0x1f;
                        a = (c & 0x1) ? 0xff000000 : 0x0;
                        
                        line[x] = (a | (r << (16 + 3)) | (g << (8 + 3)) | (b << 3));
                    }
                }
            }
                break;
                
            case DAVA::FORMAT_RGBA4444:
            {
                DAVA::uint16 *data = (DAVA::uint16 *) image->data;
                DAVA::uint32 c;
                
                qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);
                
                // convert DAVA:RGBA4444 into Qt ARGB8888
                for (int y = 0; y < (int)image->height; y++)
                {
                    line = (QRgb *) qtImage.scanLine(y);
                    for (int x = 0; x < (int)image->width; x++)
                    {
                        DAVA::uint32 a;
                        DAVA::uint32 r;
                        DAVA::uint32 g;
                        DAVA::uint32 b;
                        
                        c = data[y * image->width + x];
                        r = (c >> 12) & 0xf;
                        g = (c >> 8) & 0xf;
                        b = (c >> 4) & 0xf;
                        a = (c & 0xf);
                        
                        line[x] = ((a << (24 + 4)) | (r << (16 + 4)) | (g << (8+4)) | (b << 4));
                    }
                }
            }
                break;
                
            case DAVA::FORMAT_RGB565:
            {
                DAVA::uint16 *data = (DAVA::uint16 *) image->data;
                DAVA::uint32 c;
                
                qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);
                
                // convert DAVA:RGBA565 into Qt ARGB8888
                for (int y = 0; y < (int)image->height; y++)
                {
                    line = (QRgb *) qtImage.scanLine(y);
                    for (int x = 0; x < (int)image->width; x++)
                    {
                        DAVA::uint32 a;
                        DAVA::uint32 r;
                        DAVA::uint32 g;
                        DAVA::uint32 b;
                        
                        c = data[y * image->width + x];
                        a = 0xff;
                        r = (c >> 11) & 0x1f;
                        g = (c >> 5) & 0x3f;
                        b = c & 0x1f;
                        
                        line[x] = ((a << 24) | (r << (16 + 3)) | (g << (8 + 2)) | (b << 3));
                    }
                }
            }
                break;
                
            case DAVA::FORMAT_A8:
            {
                DAVA::uint8 *data = (DAVA::uint8 *) image->data;
                DAVA::uint32 c;
                
                qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);
                
                // convert DAVA:RGBA565 into Qt ARGB8888
                for (int y = 0; y < (int)image->height; y++) 
                {
                    line = (QRgb *) qtImage.scanLine(y);
                    for (int x = 0; x < (int)image->width; x++) 
                    {
                        c = data[y * image->width + x];
                        line[x] = ((0xff << 24) | (c << 16) | (c << 8) | c);
                    }
                }
            }
                break;
                
            case DAVA::FORMAT_RGB888:
            {
                DAVA::uint8 *data = (DAVA::uint8 *) image->data;
                
                qtImage = QImage(image->width, image->height, QImage::Format_ARGB32);
                
                // convert DAVA:RGB888 into Qt ARGB8888
                int32 imagewidth = image->width * 3;
                for (int y = 0; y < (int)image->height; y++) 
                {
                    line = (QRgb *) qtImage.scanLine(y);
                    for (int x = 0, i = 0; x < imagewidth; x += 3, i++) 
                    {
                        DAVA::uint32 a = 0xff000000;
                        DAVA::uint32 r = data[y * imagewidth + x];
                        DAVA::uint32 g = data[y * imagewidth + x + 1];
                        DAVA::uint32 b = data[y * imagewidth + x + 2];
                        
                        line[i] = (a) | (r << 16) | (g << 8) | (b);
                    }
                }
            }
                break;
                
                
            default:
                break;
        }
    }
    
    return qtImage;
}

