/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#include "Render/RenderBase.h"
#include "Render/Texture.h"
#include "Utils/Utils.h"
#include "FileSystem/Logger.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Render/RenderManager.h"
#include "Utils/StringFormat.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/File.h"
#include "Render/D3D9Helpers.h"
#include "Render/ImageConvert.h"
#include "FileSystem/FileSystem.h"
#include "Render/OGLHelpers.h"

#if defined(__DAVAENGINE_IPHONE__) 
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#elif defined(__DAVAENGINE_MACOS__)
#include <ApplicationServices/ApplicationServices.h>
#endif //PLATFORMS

#include "Render/Image.h"
#include "Render/OGLHelpers.h"

#include "Render/TextureDescriptor.h"
#include "Render/ImageLoader.h"

#include "Render/GPUFamilyDescriptor.h"


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
	
class TextureMemoryUsageInfo
{
public:
	TextureMemoryUsageInfo()
	{
		pvrTexturesMemoryUsed = 0;
		texturesMemoryUsed = 0;
		fboMemoryUsed = 0;
	}
	
	void AllocPVRTexture(int size)
	{
		pvrTexturesMemoryUsed += size;
	}
	
	void ReleasePVRTexture(int size)
	{
		pvrTexturesMemoryUsed -= size;
	}
	
	void AllocTexture(int size)
	{
		texturesMemoryUsed += size;
	}
	
	void ReleaseTexture(int size)
	{
		texturesMemoryUsed -= size;
	}
	
	void AllocFBOTexture(int size)
	{
		fboMemoryUsed += size;
	}
	
	void ReleaseFBOTexture(int size)
	{
		fboMemoryUsed -= size;
	}
	
	// STATISTICS
	int pvrTexturesMemoryUsed;
	int texturesMemoryUsed;
	int	fboMemoryUsed;
};

eGPUFamily Texture::defaultGPU = GPU_UNKNOWN;
    
static TextureMemoryUsageInfo texMemoryUsageInfo;
	
Map<String, Texture*> Texture::textureMap;
Texture * Texture::pinkPlaceholder = 0;
static int32 textureFboCounter = 0;

// Main constructurs
Texture * Texture::Get(const FilePath & pathName)
{
	Texture * texture = NULL;
	Map<String, Texture *>::iterator it;
	it = textureMap.find(pathName.GetAbsolutePathname());
	if (it != textureMap.end())
	{
		texture = it->second;
		texture->Retain();
		return texture;
	}
	return 0;
}

Texture::Texture()
:	id(0)
,	width(0)
,	height(0)
,	format(FORMAT_INVALID)
,	depthFormat(DEPTH_NONE)
,	isRenderTarget(false)
,   loadedAsFile(GPU_UNKNOWN)
{
#ifdef __DAVAENGINE_DIRECTX9__
	saveTexture = 0;
	renderTargetModified = false;
    renderTargetAutosave = true;
#endif //#ifdef __DAVAENGINE_DIRECTX9__


#ifdef __DAVAENGINE_OPENGL__
	fboID = -1;
	rboID = -1;
#endif

	invalidater = NULL;
}

Texture::~Texture()
{
    ReleaseTextureData();
}
    
void Texture::ReleaseTextureData()
{
	RenderManager::Instance()->LockNonMain();
	if(RenderManager::Instance()->GetTexture() == this)
	{//to avoid drawing deleted textures
		RenderManager::Instance()->SetTexture(0);
	}
    
#if defined(__DAVAENGINE_OPENGL__)
    
	if(fboID != (uint32)-1)
	{
		RENDER_VERIFY(glDeleteFramebuffers(1, &fboID));
        
    }
    
//#if defined(__DAVAENGINE_ANDROID__)
//		SafeDeleteArray(savedData);
//		savedDataSize = 0;
//#endif// #if defined(__DAVAENGINE_ANDROID__)
    
	if(rboID != (uint32)-1)
	{
		RENDER_VERIFY(glDeleteRenderbuffers(1, &rboID));
	}
	
	if(id)
	{
		RENDER_VERIFY(glDeleteTextures(1, &id));
	}
#elif defined(__DAVAENGINE_DIRECTX9__)
	D3DSafeRelease(id);
	D3DSafeRelease(saveTexture);
#endif //#if defined(__DAVAENGINE_OPENGL__)
	RenderManager::Instance()->UnlockNonMain();
}


Texture * Texture::CreateTextFromData(PixelFormat format, uint8 * data, uint32 width, uint32 height, bool generateMipMaps, const char * addInfo)
{
	RenderManager::Instance()->LockNonMain();
	Texture * tx = CreateFromData(format, data, width, height, generateMipMaps);
//#if defined(__DAVAENGINE_ANDROID__)
//    tx->SaveData(format, data, width, height);
//#endif //#if defined(__DAVAENGINE_ANDROID__)
	RenderManager::Instance()->UnlockNonMain();
    
	if (!addInfo)
		tx->relativePathname = Format("Text texture %d", textureFboCounter);
	else 
		tx->relativePathname = Format("Text texture %d info:%s", textureFboCounter, addInfo);

	textureFboCounter++;
    textureMap[tx->relativePathname.GetAbsolutePathname()] = tx;
	return tx;
}
	
void Texture::TexImage(int32 level, uint32 width, uint32 height, const void * _data, uint32 dataSize)
{
#if defined(__DAVAENGINE_OPENGL__)

	int32 saveId = RenderManager::Instance()->HWglGetLastTextureID();
	RenderManager::Instance()->HWglBindTexture(id);

    RENDER_VERIFY(glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ));

	DVASSERT((0 <= format) && (format < FORMAT_COUNT));
	
    if(FORMAT_INVALID != format)
    {
		if (IsCompressedFormat(format))
        {
            RENDER_VERIFY(glCompressedTexImage2D(GL_TEXTURE_2D, level, pixelDescriptors[format].internalformat, width, height, 0, dataSize, _data));
        }
        else
        {
            RENDER_VERIFY(glTexImage2D(GL_TEXTURE_2D, level, pixelDescriptors[format].internalformat, width, height, 0, pixelDescriptors[format].format, pixelDescriptors[format].type, _data));
        }
    }
	
	if(0 != saveId)
	{
		RenderManager::Instance()->HWglBindTexture(saveId);
	}

#elif defined(__DAVAENGINE_DIRECTX9__)
	if (!id)
		return;

	D3DLOCKED_RECT rect;
	HRESULT hr = id->LockRect(level, &rect, 0, 0);
	if (FAILED(hr))
	{
		Logger::Error("[TextureDX9] Could not lock DirectX9 Texture.");
		return;
	}

	// \todo instead of hardcoding transformations, use ImageConvert.
	int32 pixelSizeInBits = GetPixelFormatSize(format);
	if (format ==  FORMAT_RGBA8888)
	{
		//int32 pitchInBytes = 

		uint8 * destBits = (uint8*)rect.pBits;
		uint8 * sourceBits = (uint8*)_data;
		for (uint32 h = 0; h < height * width; ++h)
		{
// 			uint32 r = sourceBits[0];
// 			uint32 g = sourceBits[1];
// 			uint32 b = sourceBits[2];
// 			uint32 a = sourceBits[3];
// 			
// 		// 		r = ((r * a) >> 8);
// 		// 		g = ((g * a) >> 8);
// 		// 		b = ((b * a) >> 8);
// 
// 			destBits[0] = (uint8)b; //sourceBits[3];
// 			destBits[1] = (uint8)g; //sourceBits[0];
// 			destBits[2] = (uint8)r;//sourceBits[1];
// 			destBits[3] = (uint8)a;

            destBits[0] = sourceBits[2];
            destBits[1] = sourceBits[1];
            destBits[2] = sourceBits[0];
            destBits[3] = sourceBits[3];

			destBits += 4;
			sourceBits += 4;
		}
	}else 
	{
		// pixel conversion from R4G4B4A4 (OpenGL format) => A4R4G4B4 (DirectX format)
		uint16 * destBits = (uint16*)rect.pBits;
		uint16 * sourceBits = (uint16*)_data;
		for (uint32 h = 0; h < height * width; ++h)
		{
			uint32 rgba = sourceBits[0];

			destBits[0] = ((rgba & 0xF) << 12) | (rgba >> 4);

			destBits ++;
			sourceBits ++;
		}	
	}

	id->UnlockRect(level);
#endif 
}
    
Texture * Texture::CreateFromData(PixelFormat _format, const uint8 *_data, uint32 _width, uint32 _height, bool generateMipMaps)
{
	Texture * texture = new Texture();
	if (!texture)return 0;

    RenderManager::Instance()->LockNonMain();
	
	texture->width = _width;
	texture->height = _height;
	texture->format = _format;

#if defined(__DAVAENGINE_OPENGL__)
    
    texture->GenerateID();
	texture->TexImage(0, _width, _height, _data, 0);

	int32 saveId = RenderManager::Instance()->HWglGetLastTextureID();
	RenderManager::Instance()->HWglBindTexture(texture->id);

	RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	if (generateMipMaps)
	{
        RENDER_VERIFY(glGenerateMipmap(GL_TEXTURE_2D));
        RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
        RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
    else
	{
		RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	}
	
	if (saveId != 0)
	{
		RenderManager::Instance()->HWglBindTexture(saveId);
	}
    
	
#elif defined(__DAVAENGINE_DIRECTX9__)

	texture->id = CreateTextureNative(Vector2((float32)_width, (float32)_height), texture->format, false, 0);
	texture->TexImage(0, _width, _height, _data, 0);

	// allocate only 2 levels, and reuse buffers for generation of every mipmap level
	uint8 *mipMapData = new uint8[(_width / 2) * (_height / 2) * GetPixelFormatSize(texture->format) / 8];
	uint8 *mipMapData2 = new uint8[(_width / 4) * (_height / 4) * GetPixelFormatSize(texture->format) / 8];

	const uint8 * prevMipData = _data;
	uint8 * currentMipData = mipMapData;

	int32 mipMapWidth = _width / 2;
	int32 mipMapHeight = _height / 2;

	for (uint32 i = 1; i < texture->id->GetLevelCount(); ++i)
	{
		ImageConvert::DownscaleTwiceBillinear((PixelFormat)texture->format, (PixelFormat)texture->format, 
			prevMipData, mipMapWidth << 1, mipMapHeight << 1, (mipMapWidth << 1) * GetPixelFormatSize(texture->format) / 8,
			currentMipData, mipMapWidth, mipMapHeight, mipMapWidth * GetPixelFormatSize(texture->format) / 8);

		texture->TexImage(i, mipMapWidth, mipMapHeight, currentMipData, 0);
		
		mipMapWidth  >>= 1;
		mipMapHeight >>= 1;
		
		prevMipData = currentMipData;
		currentMipData = (i & 1) ? (mipMapData2) : (mipMapData); 
	}

	SafeDeleteArray(mipMapData2);
	SafeDeleteArray(mipMapData);

#endif //#if defined(__DAVAENGINE_OPENGL__)
    
    RenderManager::Instance()->UnlockNonMain();
    
	return texture;
}		
	
void Texture::SetWrapMode(TextureWrap wrapS, TextureWrap wrapT)
{
    RenderManager::Instance()->LockNonMain();
#if defined(__DAVAENGINE_OPENGL__)
	int32 saveId = RenderManager::Instance()->HWglGetLastTextureID();
	RenderManager::Instance()->HWglBindTexture(id);
	
	GLint glWrapS = HWglConvertWrapMode(wrapS);
	RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrapS));
	
	GLint glWrapT = HWglConvertWrapMode(wrapT);
	RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrapT));

	if (saveId != 0)
	{
		RenderManager::Instance()->HWglBindTexture(saveId);
	}
#elif defined(__DAVAENGINE_DIRECTX9____)
	


#endif //#if defined(__DAVAENGINE_OPENGL__) 
    RenderManager::Instance()->UnlockNonMain();
}
	
void Texture::GenerateMipmaps()
{
	if(IsCompressedFormat(format))
    {
		return;
	}
    
	RenderManager::Instance()->LockNonMain();
    
#if defined(__DAVAENGINE_OPENGL__)

	int32 saveId = RenderManager::Instance()->HWglGetLastTextureID();
	RenderManager::Instance()->HWglBindTexture(id);
	
    RENDER_VERIFY(glGenerateMipmap(GL_TEXTURE_2D));
    RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	if (saveId != 0)
	{
		RenderManager::Instance()->HWglBindTexture(saveId);
	}
	
	
#elif defined(__DAVAENGINE_DIRECTX9__)

#endif // #if defined(__DAVAENGINE_OPENGL__)
	RenderManager::Instance()->UnlockNonMain();
}

void Texture::GeneratePixelesation()
{
	RenderManager::Instance()->LockNonMain();

#if defined(__DAVAENGINE_OPENGL__)
    
	int saveId = RenderManager::Instance()->HWglGetLastTextureID();
	RenderManager::Instance()->HWglBindTexture(id);
	
	RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    
	if (saveId != 0)
	{
		RenderManager::Instance()->HWglBindTexture(saveId);
	}
	
	
#elif defined(__DAVAENGINE_DIRECTX9__)
    
#endif // #if defined(__DAVAENGINE_OPENGL__)
	RenderManager::Instance()->UnlockNonMain();
}
    

Texture * Texture::CreateFromImage(const FilePath &pathname, DAVA::TextureDescriptor *descriptor)
{
    File *file = File::Create(pathname, File::OPEN | File::READ);
    if(!file)
    {
        Logger::Error("[Texture::CreateFromImage] Cannot open file %s", pathname.GetAbsolutePathname().c_str());
        return NULL;
    }

    Texture *texture = CreateFromImage(file, descriptor);
    SafeRelease(file);
	return texture;
}
    
Texture * Texture::CreateFromImage(File *file, TextureDescriptor *descriptor)
{
    Texture * texture = new Texture();
    if(!texture)
    {
        Logger::Error("[Texture::CreateFromImage] Cannot allocate memory for Texture");
        return NULL;
    }
    
    bool loaded = texture->LoadFromImage(file, descriptor);
    if(!loaded)
    {
        SafeRelease(texture);
        Logger::Error("[Texture::CreateFromImage] Cannot load texture from image");
    }
    return texture;
}
    
bool Texture::LoadFromImage(File *file, const TextureDescriptor *descriptor)
{
    Vector<Image *> imageSet = ImageLoader::CreateFromFile(file);
    if(0 != imageSet.size())
    {
        bool isSizeCorrect = CheckImageSize(imageSet);
        if(!isSizeCorrect)
        {
            for_each(imageSet.begin(), imageSet.end(), SafeRelease<Image>);
            return false;
        }
        

        RenderManager::Instance()->LockNonMain();
#if defined(__DAVAENGINE_OPENGL__)
        GenerateID();
#elif defined(__DAVAENGINE_DIRECTX9__)
        id = CreateTextureNative(Vector2((float32)_width, (float32)_height), texture->format, false, 0);
#endif //#if defined(__DAVAENGINE_OPENGL__)
        RenderManager::Instance()->UnlockNonMain();

        
        width = imageSet[0]->width;
        height = imageSet[0]->height;
        format = imageSet[0]->format;
        
        bool needGenerateMipMaps = descriptor->GetGenerateMipMaps() && (1 == imageSet.size());

        RenderManager::Instance()->LockNonMain();
        for(uint32 i = 0; i < (uint32)imageSet.size(); ++i)
        {
			TexImage(i, imageSet[i]->width, imageSet[i]->height, imageSet[i]->data, imageSet[i]->dataSize);
            SafeRelease(imageSet[i]);
        }
        
#if defined(__DAVAENGINE_OPENGL__)
        
        int32 saveId = RenderManager::Instance()->HWglGetLastTextureID();
        RenderManager::Instance()->HWglBindTexture(id);
        
        RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, HWglConvertWrapMode((TextureWrap)descriptor->settings.wrapModeS)));
        RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, HWglConvertWrapMode((TextureWrap)descriptor->settings.wrapModeS)));
        
        if(needGenerateMipMaps)
        {
            RENDER_VERIFY(glGenerateMipmap(GL_TEXTURE_2D));
        }
        
        RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, HWglFilterToGLFilter((TextureFilter)descriptor->settings.minFilter)));
        RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, HWglFilterToGLFilter((TextureFilter)descriptor->settings.magFilter)));
        
        RenderManager::Instance()->HWglBindTexture(saveId);
#elif defined(__DAVAENGINE_DIRECTX9__)
        
        if(needGenerateMipMaps)
        {
            // allocate only 2 levels, and reuse buffers for generation of every mipmap level
            uint8 *mipMapData = new uint8[(width / 2) * (height / 2) * GetPixelFormatSizeInBytes(format)];
            uint8 *mipMapData2 = new uint8[(width / 4) * (height / 4) * GetPixelFormatSizeInBytes(format)];
            
            const uint8 * prevMipData = _data;
            uint8 * currentMipData = mipMapData;
            
            int32 mipMapWidth = width / 2;
            int32 mipMapHeight = height / 2;
            
            for (uint32 i = 1; i < id->GetLevelCount(); ++i)
            {
                ImageConvert::DownscaleTwiceBillinear(format, format,
                                                      prevMipData, mipMapWidth << 1, mipMapHeight << 1, (mipMapWidth << 1) * GetPixelFormatSizeInBytes(format),
                                                      currentMipData, mipMapWidth, mipMapHeight, mipMapWidth * GetPixelFormatSizeInBytes(format));
                
                TexImage(i, mipMapWidth, mipMapHeight, currentMipData, 0);
                
                mipMapWidth  >>= 1;
                mipMapHeight >>= 1;
                
                prevMipData = currentMipData;
                currentMipData = (i & 1) ? (mipMapData2) : (mipMapData);
            }
            
            SafeDeleteArray(mipMapData2);
            SafeDeleteArray(mipMapData);
        }
#endif //#if defined(__DAVAENGINE_OPENGL__)
        
        RenderManager::Instance()->UnlockNonMain();
        return true;
    }
    return false;
}
    
bool Texture::CheckImageSize(const Vector<DAVA::Image *> &imageSet)
{
    for (int32 i = 0; i < (int32)imageSet.size(); ++i)
    {
        if(!IsPowerOf2(imageSet[i]->GetWidth()) || !IsPowerOf2(imageSet[i]->GetHeight()))
        {
            return false;
        }
    }
    
    return true;
}

bool Texture::IsCompressedFormat(PixelFormat format)
{
	bool retValue =  false;
	if(FORMAT_INVALID != format)
    {
        if (FORMAT_PVR2 == format ||
			FORMAT_PVR4 == format ||
			(format >= FORMAT_DXT1 && format <= FORMAT_DXT5NM) ||
			FORMAT_ETC1 == format ||
			FORMAT_ATC_RGB == format ||
			FORMAT_ATC_RGBA_EXPLICIT_ALPHA == format ||
			FORMAT_ATC_RGBA_INTERPOLATED_ALPHA == format)
        {
            retValue = true;
        }
    }
	return retValue;
}

void Texture::LoadMipMapFromFile(int32 level, const FilePath & pathname)
{
    DVASSERT(false);
    return;

    
//	Image * image = Image::CreateFromFile(pathname);
//	if (image->GetPixelFormat() != format)
//	{
//		Logger::Error("Texture::LoadMipMapFromFile - format of file texture different from this texture"); 
//		SafeRelease(image);
//		return;
//	}
//	TexImage(level, image->GetWidth(), image->GetHeight(), image->GetData(), 0);
//	SafeRelease(image);
}
	
Texture * Texture::CreateFromFile(const FilePath & pathName)
{
	Texture * texture = PureCreate(pathName);
	if(!texture)
	{
		texture = GetPinkPlaceholder();
	}

	return texture;
}

Texture * Texture::PureCreate(const FilePath & pathName)
{
	if(pathName.IsEmpty() || pathName.GetType() == FilePath::PATH_IN_MEMORY)
		return NULL;

    //TODO::temporary workaround to optimize old scenes loading
    FilePath descriptorPathname = TextureDescriptor::GetDescriptorPathname(pathName);
    Texture * texture = Texture::Get(descriptorPathname);
//    Texture * texture = Texture::Get(pathName);
	if (texture) return texture;
    //ENDOF TODO
    
    TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(pathName);
    if(!descriptor) return NULL;
    
	// TODO: add check that pathName
    if(pathName.IsEqualToExtension(TextureDescriptor::GetDescriptorExtension()))
    {
		texture = CreateFromDescriptor(descriptor);
    }
    else
    {
        eGPUFamily gpuFamily = GetFormatForLoading(GPUFamilyDescriptor::GetGPUForPathname(pathName), descriptor);
        if((GPU_UNKNOWN == defaultGPU) || IsLoadAvailable(gpuFamily, descriptor))
        {
            texture = CreateFromImage(pathName, descriptor);
            if(texture)
            {
                texture->loadedAsFile = gpuFamily;
            }
        }
    }

    if(texture)
    {
		texture->relativePathname = descriptor->pathname;
        textureMap[texture->relativePathname.GetAbsolutePathname()] = texture;
        texture->SetWrapMode((TextureWrap)descriptor->settings.wrapModeS, (TextureWrap)descriptor->settings.wrapModeT);
    }
    
    SafeRelease(descriptor);
	return texture;
}
    
Texture * Texture::CreateFromDescriptor(TextureDescriptor *descriptor)
{
    Texture * texture = NULL;
    
    eGPUFamily gpuForLoading = (GPU_UNKNOWN == defaultGPU) ? (eGPUFamily)descriptor->exportedAsGpuFamily : defaultGPU;
    gpuForLoading = GetFormatForLoading(gpuForLoading, descriptor);
    if((GPU_UNKNOWN == defaultGPU) || IsLoadAvailable(gpuForLoading, descriptor))
    {
        FilePath imagePathname = GetActualFilename(descriptor, gpuForLoading);
        texture = CreateFromImage(imagePathname, descriptor);
        if(texture)
        {
            texture->loadedAsFile = gpuForLoading;
        }
    }
    
    return texture;
}

FilePath Texture::GetActualFilename(const TextureDescriptor *descriptor, const eGPUFamily gpuFamily)
{
    return GPUFamilyDescriptor::CreatePathnameForGPU(descriptor, gpuFamily);
}

	
TextureDescriptor * Texture::CreateDescriptor() const
{
    if(!isRenderTarget)
    {
        return TextureDescriptor::CreateFromFile(relativePathname);
    }

    return NULL;
}
    
void Texture::Reload()
{
    ReloadAs(loadedAsFile);
}
    
void Texture::ReloadAs(eGPUFamily gpuFamily)
{
	TextureDescriptor *descriptor = CreateDescriptor();
	ReloadAs(gpuFamily, descriptor);
	SafeRelease(descriptor);
}

void Texture::ReloadAs(eGPUFamily gpuFamily, const TextureDescriptor *descriptor)
{
    ReleaseTextureData();
	
	DVASSERT(NULL != descriptor);
    
	eGPUFamily gpuForLoading = GetFormatForLoading(gpuFamily, descriptor);
    FilePath imagePathname = GPUFamilyDescriptor::CreatePathnameForGPU(descriptor, gpuForLoading);
    
    File *file = File::Create(imagePathname, File::OPEN | File::READ);

    bool loaded = false;
    if(descriptor && file && IsLoadAvailable(gpuForLoading, descriptor))
    {
        loaded = LoadFromImage(file, descriptor);
    }
    
    if(!loaded)
    {
        width = 16;
		height = 16;
        format = FORMAT_RGBA8888;
        
        RenderManager::Instance()->LockNonMain();
#if defined(__DAVAENGINE_OPENGL__)
        GenerateID();
#elif defined(__DAVAENGINE_DIRECTX9__)
        id = CreateTextureNative(Vector2((float32)width, (float32)height), format, false, 0);
#endif //#if defined(__DAVAENGINE_OPENGL__)
        RenderManager::Instance()->UnlockNonMain();
        
        Logger::Error("[Texture::ReloadAs] Cannot reload from file %s", imagePathname.GetAbsolutePathname().c_str());
        
        
        uint32 dataSize = width * height * GetPixelFormatSizeInBytes(format);
		uint8 * data = new uint8[dataSize];
		uint32 pink = 0xffff00ff;
		uint32 gray = 0xff7f7f7f;
		bool pinkOrGray = false;
        
		uint32 * writeData = (uint32*)data;
		for(uint32 w = 0; w < width; ++w)
		{
			pinkOrGray = !pinkOrGray;
			for(uint32 h = 0; h < height; ++h)
			{
				*writeData++ = pinkOrGray ? pink : gray;
				pinkOrGray = !pinkOrGray;
			}
		}
        
        TexImage(0, width, height, data, dataSize);
        GeneratePixelesation();
    }
    else
    {
        loadedAsFile = gpuForLoading;
    }
    
    SafeRelease(file);
}
    
bool Texture::IsLoadAvailable(const eGPUFamily gpuFamily, const TextureDescriptor *descriptor)
{
    if(descriptor->IsCompressedFile())
    {
        return true;
    }
    
    DVASSERT(gpuFamily < GPU_FAMILY_COUNT);
    
    if(gpuFamily != GPU_UNKNOWN && descriptor->compression[gpuFamily].format == FORMAT_INVALID)
    {
        return false;
    }
    
    return true;
}

    
int32 Texture::Release()
{
	if(GetRetainCount() == 1)
	{
		textureMap.erase(relativePathname.GetAbsolutePathname());
	}
	return BaseObject::Release();
}
	
Texture * Texture::CreateFBO(uint32 w, uint32 h, PixelFormat format, DepthFormat _depthFormat)
{
	int i;
	int dx = w;
	if(dx < 16)
	{
		dx = 16;
	}
	else if((dx != 1) && (dx & (dx - 1))) 
	{
		i = 1;
		while(i < dx)
			i *= 2;
		dx = i;
	}
	int dy = h;
	if(dy < 16)
	{
		dy = 16;
	}
	else if((dy != 1) && (dy & (dy - 1))) 
	{
		i = 1;
		while(i < dy)
			i *= 2;
		dy = i;
	}
	
	RenderManager::Instance()->LockNonMain();
	
#if defined(__DAVAENGINE_OPENGL__)

	GLint saveFBO = RenderManager::Instance()->HWglGetLastFBO();
	GLint saveTexture = RenderManager::Instance()->HWglGetLastTextureID();

	Texture *tx = Texture::CreateFromData(format, NULL, dx, dy, false);
    
	DVASSERT(tx);

	tx->depthFormat = _depthFormat;
	 
	RenderManager::Instance()->HWglBindTexture(tx->id);
	
	RENDER_VERIFY(glGenFramebuffers(1, &tx->fboID));
	RenderManager::Instance()->HWglBindFBO(tx->fboID);
    
    if(DEPTH_RENDERBUFFER == _depthFormat)
    {
        glGenRenderbuffers(1, &tx->rboID);
        glBindRenderbuffer(GL_RENDERBUFFER, tx->rboID);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, dx, dy);
    }
		
	RENDER_VERIFY(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tx->id, 0));
    
    if(DEPTH_RENDERBUFFER == _depthFormat)
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, tx->rboID);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, tx->rboID);
    }

		
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		Logger::Error("glCheckFramebufferStatus: %d", status);
	}
				
	RenderManager::Instance()->HWglBindFBO(saveFBO);
				
	if(saveTexture)
	{
		RenderManager::Instance()->HWglBindTexture(saveTexture);
	}
#elif defined(__DAVAENGINE_DIRECTX9__)
	
	// TODO: Create FBO
	Texture * tx = new Texture();
	if (!tx)return 0;

	tx->width = dx;
	tx->height = dy;
	tx->format = format;

	tx->id = CreateTextureNative(Vector2((float32)tx->width, (float32)tx->height), tx->format, true, 0);

#endif 

	RenderManager::Instance()->UnlockNonMain();


	tx->isRenderTarget = true;
	tx->relativePathname = Format("FBO texture %d", textureFboCounter);
    textureMap[tx->relativePathname.GetAbsolutePathname()] = tx;
	
	textureFboCounter++;
	
	return tx;
}

	
void Texture::DumpTextures()
{
	int32 allocSize = 0;
	int32 cnt = 0;
	Logger::Info("============================================================");
	Logger::Info("--------------- Currently allocated textures ---------------");
	for(Map<String, Texture *>::iterator it = textureMap.begin(); it != textureMap.end(); ++it)
	{
		Texture *t = it->second;
		Logger::Info("%s with id %d (%dx%d) retainCount: %d debug: %s format: %s", t->relativePathname.GetAbsolutePathname().c_str(), t->id, t->width, t->height, t->GetRetainCount(), t->debugInfo.c_str(), GetPixelFormatString(t->format));
		cnt++;
        
        DVASSERT((0 <= t->format) && (t->format < FORMAT_COUNT));
        if(FORMAT_INVALID != t->format)
        {
            allocSize += t->width * t->height * GetPixelFormatSizeInBytes(t->format);
        }
	}
	Logger::Info("      Total allocated textures %d    memory size %d", cnt, allocSize);
	Logger::Info("============================================================");
}
	
PixelFormat Texture::defaultRGBAFormat = FORMAT_RGBA8888;

void Texture::SetDefaultRGBAFormat(PixelFormat format)
{
	defaultRGBAFormat = format;
}

PixelFormat Texture::GetDefaultRGBAFormat()
{
	return defaultRGBAFormat;
}

void Texture::SetDebugInfo(const String & _debugInfo)
{
#if defined(__DAVAENGINE_DEBUG__)
	debugInfo = _debugInfo;	
#endif
}
	
#if defined(__DAVAENGINE_ANDROID__)
	
void Texture::Lost()
{
	RenderResource::Lost();
	
	RenderManager::Instance()->LockNonMain();
	
	if(RenderManager::Instance()->GetTexture() == this)
	{//to avoid drawing deleted textures
		RenderManager::Instance()->SetTexture(0);
	}
	
	if(fboID != (uint32)-1)
	{
		RENDER_VERIFY(glDeleteFramebuffers(1, &fboID));
		fboID = -1;
	}
	
	if(id)
	{
		RENDER_VERIFY(glDeleteTextures(1, &id));
		id = 0;
	}
	
	RenderManager::Instance()->UnlockNonMain();
}

void Texture::Invalidate()
{
	RenderResource::Invalidate();
	
	DVASSERT(id == 0 && "Texture always invalidated");
	if (id)
	{
		return;
	}
	
	if (relativePathname.GetType() == FilePath::PATH_IN_FILESYSTEM ||
		relativePathname.GetType() == FilePath::PATH_IN_RESOURCES ||
		relativePathname.GetType() == FilePath::PATH_IN_DOCUMENTS)
	{
		Reload();
	}
	else if (relativePathname.GetType() == FilePath::PATH_IN_MEMORY)
	{
		if (invalidater)
			invalidater->InvalidateTexture(this);
	}
	else if (this == pinkPlaceholder)
	{
		uint32 width = 16;
		uint32 height = 16;
		uint8 * data = new uint8[width*height*4];
		uint32 pink = 0xffff00ff;
		uint32 gray = 0xff7f7f7f;
		bool pinkOrGray = false;
		
		uint32 * writeData = (uint32*)data;
		for(uint32 w = 0; w < width; ++w)
		{
			pinkOrGray = !pinkOrGray;
			for(uint32 h = 0; h < height; ++h)
			{
				*writeData++ = pinkOrGray ? pink : gray;
				pinkOrGray = !pinkOrGray;
			}
		}
		
		GenerateID();
		TexImage(0, width, height, data, 0);
		
		int32 saveId = RenderManager::Instance()->HWglGetLastTextureID();
		RenderManager::Instance()->HWglBindTexture(id);
		
		RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		RENDER_VERIFY(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		
		SafeDelete(data);
	}
}
#endif

Image * Texture::ReadDataToImage()
{
    Image *image = Image::Create(width, height, format);
    uint8 *imageData = image->GetData();
    
#if defined(__DAVAENGINE_OPENGL__)
    
    RenderManager::Instance()->LockNonMain();
    
    int32 saveFBO = RenderManager::Instance()->HWglGetLastFBO();
    int32 saveId = RenderManager::Instance()->HWglGetLastTextureID();

	RenderManager::Instance()->HWglBindTexture(id);
    
    DVASSERT((0 <= format) && (format < FORMAT_COUNT));
    if(FORMAT_INVALID != format)
    {
		RENDER_VERIFY(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        RENDER_VERIFY(glReadPixels(0, 0, width, height, pixelDescriptors[format].format, pixelDescriptors[format].type, (GLvoid *)imageData));
    }

    RenderManager::Instance()->HWglBindFBO(saveFBO);
    RenderManager::Instance()->HWglBindTexture(saveId);
    
    RenderManager::Instance()->UnlockNonMain();
    
#endif //#if defined(__DAVAENGINE_OPENGL__)
    
    return image; 
}

Image * Texture::CreateImageFromMemory()
{
    Image *image = NULL;
    if(isRenderTarget)
    {
        Sprite *renderTarget = Sprite::CreateFromTexture(this, 0, 0, (float32)width, (float32)height);
        RenderManager::Instance()->SetRenderTarget(renderTarget);
        
        image = ReadDataToImage();
            
        RenderManager::Instance()->RestoreRenderTarget();
        
        SafeRelease(renderTarget);
    }
    else
    {
        Sprite *renderTarget = Sprite::CreateAsRenderTarget((float32)width, (float32)height, format);
        RenderManager::Instance()->SetRenderTarget(renderTarget);

		Sprite *drawTexture = Sprite::CreateFromTexture(this, 0, 0, (float32)width, (float32)height);

        drawTexture->SetPosition(0, 0);
        drawTexture->Draw();

        RenderManager::Instance()->RestoreRenderTarget();
        
        image = renderTarget->GetTexture()->CreateImageFromMemory();

        SafeRelease(renderTarget);
        SafeRelease(drawTexture);
    }
        
    return image;
}
	
const Map<String, Texture*> & Texture::GetTextureMap()
{
    return textureMap;
}

int32 Texture::GetDataSize() const
{
    DVASSERT((0 <= format) && (format < FORMAT_COUNT));
    
    int32 allocSize = width * height * GetPixelFormatSizeInBytes(format);
    return allocSize;
}

Texture * Texture::GetPinkPlaceholder()
{
	if(pinkPlaceholder)
	{
		return SafeRetain(pinkPlaceholder);
	}
	else
	{
		uint32 width = 16;
		uint32 height = 16;
		uint8 * data = new uint8[width*height*4];
		uint32 pink = 0xffff00ff;
		uint32 gray = 0xff7f7f7f;
		bool pinkOrGray = false;

		uint32 * writeData = (uint32*)data;
		for(uint32 w = 0; w < width; ++w)
		{
			pinkOrGray = !pinkOrGray;
			for(uint32 h = 0; h < height; ++h)
			{
				*writeData++ = pinkOrGray ? pink : gray;
				pinkOrGray = !pinkOrGray;
			}
		}

		pinkPlaceholder = Texture::CreateFromData(FORMAT_RGBA8888, data, width, height, false);
		SafeDelete(data);

		return SafeRetain(pinkPlaceholder);
	}
}

void Texture::ReleasePinkPlaceholder()
{
	SafeRelease(pinkPlaceholder);
}

bool Texture::IsPinkPlaceholder()
{
	return ((pinkPlaceholder != 0) && (pinkPlaceholder == this));
}

PixelFormatDescriptor Texture::pixelDescriptors[FORMAT_COUNT];
void Texture::InitializePixelFormatDescriptors()
{
    SetPixelDescription(FORMAT_INVALID, String("WRONG FORMAT"), 0, 0, 0, 0);
    SetPixelDescription(FORMAT_RGBA8888, String("RGBA8888"), 32, GL_UNSIGNED_BYTE, GL_RGBA, GL_RGBA);
    SetPixelDescription(FORMAT_RGBA5551, String("RGBA5551"), 16, GL_UNSIGNED_SHORT_5_5_5_1, GL_RGBA, GL_RGBA);
    SetPixelDescription(FORMAT_RGBA4444, String("RGBA4444"), 16, GL_UNSIGNED_SHORT_4_4_4_4, GL_RGBA, GL_RGBA);
    SetPixelDescription(FORMAT_RGB888, String("RGB888"), 24, GL_UNSIGNED_BYTE, GL_RGB, GL_RGB);
    SetPixelDescription(FORMAT_RGB565, String("RGB565"), 16, GL_UNSIGNED_SHORT_5_6_5, GL_RGB, GL_RGB);
    SetPixelDescription(FORMAT_A8, String("A8"), 8, GL_UNSIGNED_BYTE, GL_ALPHA, GL_ALPHA);
    SetPixelDescription(FORMAT_A16, String("A16"), 16, GL_UNSIGNED_SHORT, GL_ALPHA, GL_ALPHA);
    
#if defined (GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG)
    SetPixelDescription(FORMAT_PVR4, String("PVR4"), 4, GL_UNSIGNED_BYTE, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG);
#else
    SetPixelDescription(FORMAT_PVR4, String("PVR4"), 4, 0, 0, 0);
#endif

    
#if defined (GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG)
    SetPixelDescription(FORMAT_PVR2, String("PVR2"), 2, GL_UNSIGNED_BYTE, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG);
#else
    SetPixelDescription(FORMAT_PVR2, String("PVR2"), 2, 0, 0, 0);
#endif

#if defined (GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
	SetPixelDescription(FORMAT_DXT1,     "DXT1", 4, GL_UNSIGNED_BYTE, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
	SetPixelDescription(FORMAT_DXT1NM, "DXT1nm", 4, GL_UNSIGNED_BYTE, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
#else
	SetPixelDescription(FORMAT_DXT1,     "DXT1", 4, 0, 0, 0);
	SetPixelDescription(FORMAT_DXT1NM, "DXT1nm", 4, 0, 0, 0);
#endif

#if defined (GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
	SetPixelDescription(FORMAT_DXT1A,   "DXT1a", 4, GL_UNSIGNED_BYTE, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
#else
	SetPixelDescription(FORMAT_DXT1A,   "DXT1a", 4, 0, 0, 0);
#endif

#if defined (GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
	SetPixelDescription(FORMAT_DXT3,     "DXT3", 8, GL_UNSIGNED_BYTE, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT);
#else
	SetPixelDescription(FORMAT_DXT3,     "DXT3", 8, 0, 0, 0);
#endif

#if defined (GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
	SetPixelDescription(FORMAT_DXT5,     "DXT5", 8, GL_UNSIGNED_BYTE, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
	SetPixelDescription(FORMAT_DXT5NM, "DXT5nm", 8, GL_UNSIGNED_BYTE, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
#else
	SetPixelDescription(FORMAT_DXT5,     "DXT5", 8, 0, 0, 0);
	SetPixelDescription(FORMAT_DXT5NM, "DXT5nm", 8, 0, 0, 0);
#endif

    SetPixelDescription(FORMAT_RGBA16161616, String("RGBA16161616"), 64, GL_HALF_FLOAT, GL_RGBA, GL_RGBA);
    SetPixelDescription(FORMAT_RGBA32323232, String("RGBA32323232"), 128, GL_FLOAT, GL_RGBA, GL_RGBA);

#if defined (GL_ETC1_RGB8_OES)
	SetPixelDescription(FORMAT_ETC1,     "ETC1", 8, GL_UNSIGNED_BYTE, GL_ETC1_RGB8_OES, GL_ETC1_RGB8_OES);
#else
	SetPixelDescription(FORMAT_ETC1,     "ETC1", 8, 0, 0, 0);
#endif
	
	
#if defined (GL_ATC_RGB_AMD)
	SetPixelDescription(FORMAT_ATC_RGB, "ATC_RGB", 4, GL_UNSIGNED_BYTE, GL_ATC_RGB_AMD, GL_ATC_RGB_AMD);
#else
	SetPixelDescription(FORMAT_ATC_RGB, "ATC_RGB", 4, 0, 0, 0);
#endif
	
#if defined (GL_ATC_RGBA_EXPLICIT_ALPHA_AMD)
	SetPixelDescription(FORMAT_ATC_RGBA_EXPLICIT_ALPHA, "ATC_RGBA_EXPLICIT_ALPHA", 8, GL_UNSIGNED_BYTE, GL_ATC_RGBA_EXPLICIT_ALPHA_AMD, GL_ATC_RGBA_EXPLICIT_ALPHA_AMD);
#else
	SetPixelDescription(FORMAT_ATC_RGBA_EXPLICIT_ALPHA, "ATC_RGBA_EXPLICIT_ALPHA", 8, 0, 0, 0);
#endif

#if defined (GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD)
	SetPixelDescription(FORMAT_ATC_RGBA_INTERPOLATED_ALPHA, "ATC_RGBA_INTERPOLATED_ALPHA", 8, GL_UNSIGNED_BYTE, GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD, GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD);
#else
	SetPixelDescription(FORMAT_ATC_RGBA_INTERPOLATED_ALPHA, "ATC_RGBA_INTERPOLATED_ALPHA", 8, 0, 0, 0);
#endif
}

void Texture::SetPixelDescription(PixelFormat index, const String &name, int32 size, GLenum type, GLenum format, GLenum internalFormat)
{
    DVASSERT((0 <= index) && (index < FORMAT_COUNT));
    
    pixelDescriptors[index].formatID = index;
    pixelDescriptors[index].name = name;
    pixelDescriptors[index].pixelSize = size;
    pixelDescriptors[index].format = format;
    pixelDescriptors[index].internalformat = internalFormat;
    pixelDescriptors[index].type = type;
}

PixelFormatDescriptor Texture::GetPixelFormatDescriptor(PixelFormat formatID)
{
    DVASSERT((0 <= formatID) && (formatID < FORMAT_COUNT));
    return pixelDescriptors[formatID];
}

int32 Texture::GetPixelFormatSizeInBits(PixelFormat format)
{
    DVASSERT((0 < format) && (format < FORMAT_COUNT));
    return pixelDescriptors[format].pixelSize;
}

int32 Texture::GetPixelFormatSizeInBytes(PixelFormat format)
{
    return GetPixelFormatSizeInBits(format) / 8;
}



const char * Texture::GetPixelFormatString(PixelFormat format)
{
    DVASSERT((0 <= format) && (format < FORMAT_COUNT));
    return pixelDescriptors[format].name.c_str();
}

PixelFormat Texture::GetPixelFormatByName(const String &formatName)
{
    for(int32 i = 0; i < FORMAT_COUNT; ++i)
    {
        if(0 == CompareCaseInsensitive(formatName, pixelDescriptors[i].name))
        {
            return pixelDescriptors[i].formatID;
        }
    }
    
    return FORMAT_INVALID;
}

    
    
void Texture::GenerateID()
{
#if defined(__DAVAENGINE_OPENGL__)
	for (int32 i = 0; i < 10; i++)
	{
		glGenTextures(1, &id);
		if(0 != id)
		{
			break;
		}
		Logger::Error("TEXTURE %d GENERATE ERROR: %d", i, glGetError());
	}
#endif //#if defined(__DAVAENGINE_OPENGL__)

}

#if defined (__DAVAENGINE_OPENGL__)
GLint Texture::HWglConvertWrapMode(TextureWrap wrap)
{
    switch(wrap)
    {
        case WRAP_CLAMP_TO_EDGE:
            return GL_CLAMP_TO_EDGE;
        case WRAP_REPEAT:
            return GL_REPEAT;
    };
    
    return 0;
}
    
GLint Texture::HWglFilterToGLFilter(TextureFilter filter)
{
    switch (filter)
    {
        case FILTER_NEAREST:
            return GL_NEAREST;
            
        case FILTER_LINEAR:
            return GL_LINEAR;

        case FILTER_NEAREST_MIPMAP_NEAREST:
            return GL_NEAREST_MIPMAP_NEAREST;

        case FILTER_LINEAR_MIPMAP_NEAREST:
            return GL_LINEAR_MIPMAP_NEAREST;

        case FILTER_NEAREST_MIPMAP_LINEAR:
            return GL_NEAREST_MIPMAP_LINEAR;

        case FILTER_LINEAR_MIPMAP_LINEAR:
            return GL_LINEAR_MIPMAP_LINEAR;

		default:
			DVASSERT(0 && "Wrong filter");
			break;
    }	
    
    return GL_NEAREST;
}
    
    
#endif //#if defined (__DAVAENGINE_OPENGL__)
    
    
void Texture::SetDefaultGPU(eGPUFamily gpuFamily)
{
    defaultGPU = gpuFamily;
}

eGPUFamily Texture::GetDefaultGPU()
{
    return defaultGPU;
}

    
eGPUFamily Texture::GetFormatForLoading(const eGPUFamily requestedGPU, const TextureDescriptor *descriptor)
{
    if(descriptor->IsCompressedFile())
        return (eGPUFamily)descriptor->exportedAsGpuFamily;
    
    return requestedGPU;
}

void Texture::SetInvalidater(TextureInvalidater* invalidater)
{
	this->invalidater = invalidater;
}


};
