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

#include "Render/RenderBase.h"
#include "Render/Texture.h"
#include "Utils/Utils.h"
#include "FileSystem/Logger.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Render/Renderer.h"
#include "Utils/StringFormat.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Render/RenderHelper.h"
#include "Render/RenderCallbacks.h"

#if defined(__DAVAENGINE_IPHONE__) 
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#elif defined(__DAVAENGINE_MACOS__)
#include <ApplicationServices/ApplicationServices.h>
#endif //PLATFORMS

#include "Render/Image/ImageSystem.h"
#include "Render/Image/ImageConvert.h"

#include "Render/TextureDescriptor.h"
#include "Render/GPUFamilyDescriptor.h"
#include "Math/MathHelpers.h"
#include "Concurrency/LockGuard.h"



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
namespace Validator
{
bool IsFormatSupported(PixelFormat format)
{
    const auto& formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(format);
    return formatDescriptor.isHardwareSupported;
}

bool AreImagesSquare(const Vector<DAVA::Image*>& imageSet)
{
    for (int32 i = 0; i < (int32)imageSet.size(); ++i)
    {
        if (!IsPowerOf2(imageSet[i]->GetWidth()) || !IsPowerOf2(imageSet[i]->GetHeight()))
        {
            return false;
        }
    }

    return true;
}

bool AreImagesCorrectForTexture(const Vector<DAVA::Image*>& imageSet)
{
    if (0 == imageSet.size())
    {
        Logger::Error("[TextureValidator] Loaded images count is zero");
        return false;
    }

    auto format = imageSet[0]->format;
    if (!IsFormatSupported(format))
    {
        Logger::Error("[TextureValidator] Format %d is unsupported", format);
        return false;
    }

    bool isSizeCorrect = Validator::AreImagesSquare(imageSet);
    if (!isSizeCorrect)
    {
        Logger::Error("[TextureValidator] Size if loaded images is invalid (not power of 2)");
        return false;
    }

    return true;
}
}

Array<String, Texture::CUBE_FACE_COUNT> Texture::FACE_NAME_SUFFIX =
{{
    String("_px"),
    String("_nx"),
    String("_py"),
    String("_ny"),
    String("_pz"),
    String("_nz")
}};

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

eGPUFamily Texture::defaultGPU = GPU_ORIGIN;
    
static TextureMemoryUsageInfo texMemoryUsageInfo;
	
TexturesMap Texture::textureMap;

Mutex Texture::textureMapMutex;

static int32 textureFboCounter = 0;

bool Texture::pixelizationFlag = false;

// Main constructors
Texture * Texture::Get(const FilePath & pathName)
{
    LockGuard<Mutex> guard(textureMapMutex);

	Texture* texture = nullptr;
	TexturesMap::iterator it = textureMap.find(FILEPATH_MAP_KEY(pathName));
	if (it != textureMap.end())
	{
		texture = it->second;
		texture->Retain();
	}
    return texture;
}

void Texture::AddToMap(Texture *tex)
{
    if (!tex->texDescriptor->pathname.IsEmpty())
    {
        textureMapMutex.Lock();
        DVASSERT(textureMap.find(FILEPATH_MAP_KEY(tex->texDescriptor->pathname)) == textureMap.end());
		textureMap[FILEPATH_MAP_KEY(tex->texDescriptor->pathname)] = tex;
        textureMapMutex.Unlock();
    }
}

    
Texture::Texture()
:	width(0)
,	height(0)
,	loadedAsFile(GPU_ORIGIN)
,	state(STATE_INVALID)
,	textureType(rhi::TEXTURE_TYPE_2D)
,	isRenderTarget(false)
,	isPink(false)
{
    texDescriptor = new TextureDescriptor;
    RenderCallbacks::RegisterResourceRestoreCallback(MakeFunction(this, &Texture::RestoreRenderResource));
}

Texture::~Texture()
{
    RenderCallbacks::UnRegisterResourceRestoreCallback(MakeFunction(this, &Texture::RestoreRenderResource));    
    ReleaseTextureData();
	SafeDelete(texDescriptor);
}
    
void Texture::ReleaseTextureData()
{
    if (handle.IsValid())
        rhi::DeleteTexture(handle);
    handle = rhi::HTexture(rhi::InvalidHandle);

	state = STATE_INVALID;
    isRenderTarget = false;
}



Texture * Texture::CreateTextFromData(PixelFormat format, uint8 * data, uint32 width, uint32 height, bool generateMipMaps, const char * addInfo)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	Texture * tx = CreateFromData(format, data, width, height, generateMipMaps);
    
	if (!addInfo)
    {
        tx->texDescriptor->pathname = Format("Text texture %d", textureFboCounter);
    }
	else
    {
        tx->texDescriptor->pathname = Format("Text texture %d info:%s", textureFboCounter, addInfo);
    }
    AddToMap(tx);
    
	textureFboCounter++;
	return tx;
}
	
void Texture::TexImage(int32 level, uint32 width, uint32 height, const void * _data, uint32 dataSize, uint32 cubeFaceId)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    rhi::UpdateTexture(handle, _data, level, (rhi::TextureFace)cubeFaceId);
}
    
Texture * Texture::CreateFromData(PixelFormat _format, const uint8 *_data, uint32 _width, uint32 _height, bool generateMipMaps)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	Image *image = Image::CreateFromData(_width, _height, _format, _data);
	if (nullptr == image) 
		return nullptr;

	Texture * texture = new Texture();
	texture->texDescriptor->Initialize(rhi::TEXADDR_CLAMP, generateMipMaps);
    
    Vector<Image *> *images = new Vector<Image *>();
    images->push_back(image);
	
    texture->SetParamsFromImages(images);
	texture->FlushDataToRenderer(images);

	return texture;
}
    
Texture * Texture::CreateFromData(Image *image, bool generateMipMaps)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	Texture * texture = new Texture();
    texture->texDescriptor->Initialize(rhi::TEXADDR_CLAMP, generateMipMaps);
    
    Vector<Image *> *images = new Vector<Image *>();
    image->Retain();
    images->push_back(image);
	
    texture->SetParamsFromImages(images);
	texture->FlushDataToRenderer(images);
    
	return texture;
}

	
void Texture::SetWrapMode(rhi::TextureAddrMode wrapU, rhi::TextureAddrMode wrapV, rhi::TextureAddrMode wrapW)
{
    samplerState.addrU = wrapU;
    samplerState.addrV = wrapV;
    samplerState.addrW = wrapW;
}

void Texture::SetMinMagFilter(rhi::TextureFilter minFilter, rhi::TextureFilter magFilter, rhi::TextureMipFilter mipFilter)
{
    samplerState.minFilter = minFilter;
    samplerState.magFilter = magFilter;
    samplerState.mipFilter = mipFilter;
}
	
void Texture::GenerateMipmaps()
{
    DVASSERT("Mipmap generation on fly is not supported anymore!")	
}



Texture * Texture::CreateFromImage(TextureDescriptor *descriptor, eGPUFamily gpu)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	Texture * texture = new Texture();
	texture->texDescriptor->Initialize(descriptor);

    Vector<Image *> * images = new Vector<Image *> ();
    
	bool loaded = texture->LoadImages(gpu, images);
    if (!loaded)
	{
		Logger::Error("[Texture::CreateFromImage] Cannot load texture from image. Descriptor: %s, GPU: %s",
            descriptor->pathname.GetAbsolutePathname().c_str(), GlobalEnumMap<eGPUFamily>::Instance()->ToString(gpu));

        SafeDelete(images);
		SafeRelease(texture);
		return nullptr;
	}

	texture->SetParamsFromImages(images);
	texture->FlushDataToRenderer(images);

	return texture;
}

bool Texture::LoadImages(eGPUFamily gpu, Vector<Image *> * images)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(gpu != GPU_INVALID);
    
    if (!IsLoadAvailable(gpu))
    {
        Logger::Error("[Texture::LoadImages] Load not avalible: invalid requsted GPU family (%s)", GlobalEnumMap<eGPUFamily>::Instance()->ToString(gpu));
        return false;
    }
	
    const int32 baseMipMap = GetBaseMipMap();
	if (texDescriptor->IsCubeMap() && (!GPUFamilyDescriptor::IsGPUForDevice(gpu)))
	{
        Vector<FilePath> facePathes;
        texDescriptor->GetFacePathnames(facePathes);
        
        PixelFormat imagesFormat = FORMAT_INVALID;
		for(auto i = 0; i < CUBE_FACE_COUNT; ++i)
		{
            auto & currentfacePath = facePathes[i];
            if (currentfacePath.IsEmpty())
                continue;

            Vector<Image *> faceImage;
            ImageSystem::Instance()->Load(currentfacePath, faceImage, baseMipMap);
            if (faceImage.size() == 0)
			{
                Logger::Error("[Texture::LoadImages] Cannot open file %s", currentfacePath.GetAbsolutePathname().c_str());

				ReleaseImages(images);
				return false;
			}
            
			DVASSERT(faceImage.size() == 1);

			faceImage[0]->cubeFaceID = i;
			faceImage[0]->mipmapLevel = 0;

            //cubemap formats validation
            if (FORMAT_INVALID == imagesFormat)
            {
                imagesFormat = faceImage[0]->format;
            }
            else if (imagesFormat != faceImage[0]->format)
            {
                Logger::Error("[Texture::LoadImages] Face(%s) has different pixel format(%s)", currentfacePath.GetAbsolutePathname().c_str(), PixelFormatDescriptor::GetPixelFormatString(faceImage[0]->format));
                
                ReleaseImages(images);
                return false;
            }
            //end of cubemap formats validation
            
            if (texDescriptor->GetGenerateMipMaps())
            {
                Vector<Image *> mipmapsImages = faceImage[0]->CreateMipMapsImages();
                images->insert(images->end(), mipmapsImages.begin(), mipmapsImages.end());
                SafeRelease(faceImage[0]);
            }
            else
            {
			    images->push_back(faceImage[0]);
            }
		}
	}
	else
	{
		FilePath imagePathname = texDescriptor->CreatePathnameForGPU(gpu);

        ImageSystem::Instance()->Load(imagePathname, *images, baseMipMap);
        ImageSystem::Instance()->EnsurePowerOf2Images(*images);
        if (images->size() == 1 && gpu == GPU_ORIGIN && texDescriptor->GetGenerateMipMaps())
        {
            Image * img = *images->begin();
            *images = img->CreateMipMapsImages(texDescriptor->dataSettings.GetIsNormalMap());
            SafeRelease(img);
        }
    }

    if (!Validator::AreImagesCorrectForTexture(*images))
    {
        Logger::Error("[Texture::LoadImages] cannot create texture from images");

        ReleaseImages(images);
        return false;
    }

	isPink = false;
	state = STATE_DATA_LOADED;

	return true;
}


void Texture::ReleaseImages(Vector<Image *> *images)
{
	for_each(images->begin(), images->end(), SafeRelease<Image>);
	images->clear();
}

void Texture::SetParamsFromImages(const Vector<Image *> * images)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	DVASSERT(images->size() != 0);

    Image *img = *images->begin();
	width = img->width;
	height = img->height;
	texDescriptor->format = img->format;

	textureType = (img->cubeFaceID != Texture::INVALID_CUBEMAP_FACE) ? rhi::TEXTURE_TYPE_CUBE : rhi::TEXTURE_TYPE_2D;
    
    state = STATE_DATA_LOADED;
}

void Texture::FlushDataToRenderer(Vector<Image *> * images)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(images->size() != 0);

    const PixelFormatDescriptor & formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(texDescriptor->format);
    rhi::Texture::Descriptor descriptor;
    descriptor.autoGenMipmaps = false;
    descriptor.isRenderTarget = false;
    descriptor.levelCount = ((*images)[0]->cubeFaceID == Texture::INVALID_CUBEMAP_FACE)  ? images->size()  : images->size()/6;
    descriptor.width = (*images)[0]->width;
    descriptor.height = (*images)[0]->height;
    descriptor.type = ((*images)[0]->cubeFaceID == Texture::INVALID_CUBEMAP_FACE) ? rhi::TEXTURE_TYPE_2D : rhi::TEXTURE_TYPE_CUBE;
    descriptor.format = formatDescriptor.format;


    descriptor.levelCount = (descriptor.type == rhi::TEXTURE_TYPE_CUBE) ? (uint32)images->size() / 6 : (uint32)images->size();

    for (Image * img : (*images))
        descriptor.levelCount = Max(descriptor.levelCount, img->mipmapLevel + 1);

    DVASSERT(descriptor.format != ((rhi::TextureFormat) - 1));//unsupported format
    handle = rhi::CreateTexture(descriptor);
    DVASSERT(handle != rhi::InvalidHandle);

    for (uint32 i = 0; i < (uint32)images->size(); ++i)
    {
        Image *img = (*images)[i];
        TexImage((img->mipmapLevel != (uint32)-1) ? img->mipmapLevel : i, img->width, img->height, img->data, img->dataSize, img->cubeFaceID);
    }

    samplerState.addrU = texDescriptor->drawSettings.wrapModeS;
    samplerState.addrV = texDescriptor->drawSettings.wrapModeT;
    samplerState.minFilter = texDescriptor->drawSettings.minFilter;
    samplerState.magFilter = texDescriptor->drawSettings.magFilter;
    samplerState.mipFilter = texDescriptor->drawSettings.mipFilter;

    state = STATE_VALID;

    ReleaseImages(images);
    SafeDelete(images);
}


Texture * Texture::CreateFromFile(const FilePath & pathName, const FastName &group, rhi::TextureType typeHint)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	Texture* texture = PureCreate(pathName, group);
	if (nullptr == texture)
	{
        TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(pathName);
        if (descriptor)
        {
            texture = CreatePink(descriptor->IsCubeMap() ? rhi::TEXTURE_TYPE_CUBE : typeHint);
            texture->texDescriptor->Initialize(descriptor);
            SafeDelete(descriptor);
        }
        else
        {
            texture = CreatePink(typeHint);
            texture->texDescriptor->pathname = (!pathName.IsEmpty()) ? TextureDescriptor::GetDescriptorPathname(pathName) : FilePath();
        }
        
        texture->texDescriptor->SetQualityGroup(group);
        
        AddToMap(texture);
	}

	return texture;
}

Texture * Texture::PureCreate(const FilePath & pathName, const FastName &group)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	if (pathName.IsEmpty() || (pathName.GetType() == FilePath::PATH_IN_MEMORY))
		return nullptr;

    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::TEXTURE_LOAD_ENABLED))
        return nullptr;

    FilePath descriptorPathname = TextureDescriptor::GetDescriptorPathname(pathName);
    Texture * texture = Texture::Get(descriptorPathname);
	if (texture)
		return texture;
    
    TextureDescriptor* descriptor(TextureDescriptor::CreateFromFile(descriptorPathname));
    if (nullptr == descriptor)
		return nullptr;
    
    descriptor->SetQualityGroup(group);

	eGPUFamily gpuForLoading = GetGPUForLoading(defaultGPU, descriptor);
	texture = CreateFromImage(descriptor, gpuForLoading);
	if (texture)
	{
		texture->loadedAsFile = gpuForLoading;
		AddToMap(texture);
	}

	delete descriptor;
	return texture;
}
    
void Texture::ReloadFromData(PixelFormat format, uint8 * data, uint32 _width, uint32 _height)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    rhi::HTexture oldHandle = handle;
    ReleaseTextureData();
    
    Image *image = Image::CreateFromData(_width, _height, format, data);
	if (!image) return;
    
    Vector<Image *> *images = new Vector<Image *>();
    images->push_back(image);
	
    SetParamsFromImages(images);
	FlushDataToRenderer(images);
    rhi::ReplaceTextureInAllTextureSets(oldHandle, handle);
}
    
void Texture::Reload()
{
    ReloadAs(loadedAsFile);
}
    
void Texture::ReloadAs(eGPUFamily gpuFamily)
{
    rhi::HTexture oldHandle = handle;
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(isRenderTarget == false);
    
    ReleaseTextureData();

	bool descriptorReloaded = texDescriptor->Reload();
    
	eGPUFamily gpuForLoading = GetGPUForLoading(gpuFamily, texDescriptor);
    Vector<Image *> *images = new Vector<Image *> ();
    
    bool loaded = false;
    if (descriptorReloaded && Renderer::GetOptions()->IsOptionEnabled(RenderOptions::TEXTURE_LOAD_ENABLED))
    {
	    loaded = LoadImages(gpuForLoading, images);
    }

	if (loaded)
	{
		loadedAsFile = gpuForLoading;

		SetParamsFromImages(images);
		FlushDataToRenderer(images);
	}
	else
    {
        SafeDelete(images);
        
        Logger::Error("[Texture::ReloadAs] Cannot reload from file %s", texDescriptor->pathname.GetAbsolutePathname().c_str());
        MakePink();
    }
    rhi::ReplaceTextureInAllTextureSets(oldHandle, handle);
}

    
bool Texture::IsLoadAvailable(const eGPUFamily gpuFamily) const
{
    if (texDescriptor->IsCompressedFile())
    {
        return true;
    }
    
    if (GPUFamilyDescriptor::IsGPUForDevice(gpuFamily) && texDescriptor->compression[gpuFamily].format == FORMAT_INVALID)
    {
        return false;
    }
    
    return true;
}

    
int32 Texture::Release()
{
	if (GetRetainCount() == 1)
	{
        textureMapMutex.Lock();
		textureMap.erase(FILEPATH_MAP_KEY(texDescriptor->pathname));
        textureMapMutex.Unlock();
	}
	return BaseObject::Release();
}


Texture * Texture::CreateFBO(uint32 w, uint32 h, PixelFormat format, rhi::TextureType requestedType)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    int32 dx = Max((int32)w, 8);

    EnsurePowerOf2(dx);

    int32 dy = Max((int32)h, 8);
    EnsurePowerOf2(dy);
    
    Texture *tx = new Texture();
    tx->width = dx;
    tx->height = dy;
    tx->textureType = requestedType;
    tx->texDescriptor->format = format;
    tx->samplerState.mipFilter = tx->texDescriptor->drawSettings.mipFilter = rhi::TEXMIPFILTER_NONE;
    
    const PixelFormatDescriptor & formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(format);
    rhi::Texture::Descriptor descriptor;
    descriptor.width = tx->width;
    descriptor.height = tx->height;
    descriptor.autoGenMipmaps = false;
    descriptor.isRenderTarget = true;
    descriptor.needRestore = false;
    descriptor.type = requestedType;
    descriptor.format = formatDescriptor.format;
    DVASSERT(descriptor.format != ((rhi::TextureFormat)-1));//unsupported format
    tx->handle = rhi::CreateTexture(descriptor);

    tx->isRenderTarget = true;
    tx->texDescriptor->pathname = Format("FBO texture %d", textureFboCounter);
    AddToMap(tx);

    textureFboCounter++;

    return tx;
    
}

	
void Texture::DumpTextures()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	uint32 allocSize = 0;
	int32 cnt = 0;
	Logger::FrameworkDebug("============================================================");
	Logger::FrameworkDebug("--------------- Currently allocated textures ---------------");

    textureMapMutex.Lock();
	for(TexturesMap::iterator it = textureMap.begin(); it != textureMap.end(); ++it)
	{
		Texture *t = it->second;
		Logger::FrameworkDebug("%s with id %d (%dx%d) retainCount: %d debug: %s format: %s", t->texDescriptor->pathname.GetAbsolutePathname().c_str(), (uint32)(t->handle), t->width, t->height,
								t->GetRetainCount(), t->debugInfo.c_str(), PixelFormatDescriptor::GetPixelFormatString(t->texDescriptor->format));
		cnt++;
        
        DVASSERT((0 <= t->texDescriptor->format) && (t->texDescriptor->format < FORMAT_COUNT));
        if (FORMAT_INVALID != t->texDescriptor->format)
        {
            allocSize += t->width * t->height * PixelFormatDescriptor::GetPixelFormatSizeInBits(t->texDescriptor->format);
        }
	}
    textureMapMutex.Unlock();

	Logger::FrameworkDebug("      Total allocated textures %d    memory size %d", cnt, allocSize/8);
	Logger::FrameworkDebug("============================================================");
}
	
void Texture::SetDebugInfo(const String & _debugInfo)
{
#if defined(__DAVAENGINE_DEBUG__)
	debugInfo = FastName(_debugInfo.c_str());
#endif
}	

void Texture::RestoreRenderResource()
{    
    
    if ((!handle.IsValid()) || (!NeedRestoreTexture(handle)))
        return;
	
    
    
    Vector<Image *> images;

    const FilePath& relativePathname = texDescriptor->GetSourceTexturePathname();
    if (relativePathname.GetType() == FilePath::PATH_IN_FILESYSTEM ||
        relativePathname.GetType() == FilePath::PATH_IN_RESOURCES ||
        relativePathname.GetType() == FilePath::PATH_IN_DOCUMENTS)
    {
        eGPUFamily gpuForLoading = GetGPUForLoading(loadedAsFile, texDescriptor);
        LoadImages(gpuForLoading, &images);
    }
    else if (isPink)
    {
        if (texDescriptor->IsCubeMap())
        {
            for (uint32 i = 0; i < Texture::CUBE_FACE_COUNT; ++i)
            {
                Image *img = Image::Create(width, height, FORMAT_RGBA8888);
                img->MakePink(false);
                img->cubeFaceID = i;
                img->mipmapLevel = 0;
                images.push_back(img);
            }
        }
        else
        {
            Image *img = Image::Create(width, height, FORMAT_RGBA8888);
            img->MakePink(false);
            images.push_back(img);
        }
    }

    for (uint32 i = 0; i < (uint32)images.size(); ++i)
    {
        Image *img = images[i];
        TexImage((img->mipmapLevel != (uint32)-1) ? img->mipmapLevel : i, img->width, img->height, img->data, img->dataSize, img->cubeFaceID);
    }

    ReleaseImages(&images);
    
}


Image * Texture::CreateImageFromMemory()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    Image* image = nullptr;
    
    void* mappedData = rhi::MapTexture(handle);
    image = Image::CreateFromData(width, height, texDescriptor->format, (uint8*)mappedData);
    rhi::UnmapTexture(handle);
    
    return image;
}
	
const TexturesMap & Texture::GetTextureMap()
{
    return textureMap;
}

uint32 Texture::GetDataSize() const
{
    DVASSERT((0 <= texDescriptor->format) && (texDescriptor->format < FORMAT_COUNT));
    
    uint32 allocSize = width * height * PixelFormatDescriptor::GetPixelFormatSizeInBits(texDescriptor->format) / 8;
    return allocSize;
}

Texture * Texture::CreatePink(rhi::TextureType requestedType, bool checkers)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	//we need instances for pink textures for ResourceEditor. We use it for reloading for different GPUs
	//pink textures at game is invalid situation
	Texture *tex = new Texture();
	if (rhi::TEXTURE_TYPE_CUBE == requestedType)
	{
        tex->texDescriptor->Initialize(rhi::TEXADDR_CLAMP, true);
		tex->texDescriptor->dataSettings.cubefaceFlags = 0x000000FF;
	}
	else
	{
        tex->texDescriptor->Initialize(rhi::TEXADDR_CLAMP, false);
	}

	tex->MakePink(checkers);

	return tex;
}

void Texture::MakePink(bool checkers)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    Vector<Image *> *images = new Vector<Image *> ();
	if (texDescriptor->IsCubeMap())
	{
		for(uint32 i = 0; i < Texture::CUBE_FACE_COUNT; ++i)
		{
            Image *img = Image::CreatePinkPlaceholder(checkers);
			img->cubeFaceID = i;
			img->mipmapLevel = 0;

			images->push_back(img);
		}
	}
	else
	{
		images->push_back(Image::CreatePinkPlaceholder(checkers));
	}

	SetParamsFromImages(images);
    FlushDataToRenderer(images);

    isPink = true;

    SetMinMagFilter(rhi::TEXFILTER_NEAREST, rhi::TEXFILTER_NEAREST, rhi::TEXMIPFILTER_NONE);
}
    
bool Texture::IsPinkPlaceholder()
{
	return isPink;
}
    
void Texture::SetDefaultGPU(eGPUFamily gpuFamily)
{
    defaultGPU = gpuFamily;
}

eGPUFamily Texture::GetDefaultGPU()
{
    return defaultGPU;
}

    
eGPUFamily Texture::GetGPUForLoading(const eGPUFamily requestedGPU, const TextureDescriptor *descriptor)
{
    if (descriptor->IsCompressedFile())
        return (eGPUFamily)descriptor->exportedAsGpuFamily;
    
    return requestedGPU;
}


const FilePath & Texture::GetPathname() const
{
    return texDescriptor->pathname;
}
    
void Texture::SetPathname(const FilePath& path)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    textureMapMutex.Lock();
    textureMap.erase(FILEPATH_MAP_KEY(texDescriptor->pathname));
    texDescriptor->pathname = path;
    if (!texDescriptor->pathname.IsEmpty())
    {
        DVASSERT(textureMap.find(FILEPATH_MAP_KEY(texDescriptor->pathname)) == textureMap.end());
        textureMap[FILEPATH_MAP_KEY(texDescriptor->pathname)] = this;
    }
    textureMapMutex.Unlock();
}

PixelFormat Texture::GetFormat() const
{
	return texDescriptor->format;
}

void Texture::SetPixelization(bool value)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

#if RHI_COMPLETE
    if (value == pixelizationFlag)
    {
        return;
    }

    pixelizationFlag = value;
    const TexturesMap& texturesMap = GetTextureMap();

    textureMapMutex.Lock();
    for (Map<FilePath, Texture *>::const_iterator iter = texturesMap.begin(); iter != texturesMap.end(); iter ++)
    {
        Texture* texture = iter->second;
        TextureFilter minFilter = pixelizationFlag ? FILTER_NEAREST : (TextureFilter)texture->GetDescriptor()->drawSettings.minFilter;
        TextureFilter magFilter = pixelizationFlag ? FILTER_NEAREST : (TextureFilter)texture->GetDescriptor()->drawSettings.magFilter;
        texture->SetMinMagFilter(minFilter, magFilter);
    }
    textureMapMutex.Unlock();
#endif //RHI_COMPLETE
}


int32 Texture::GetBaseMipMap() const
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if (texDescriptor->GetQualityGroup().IsValid())
    {
        const TextureQuality *curTxQuality = QualitySettingsSystem::Instance()->GetTxQuality(QualitySettingsSystem::Instance()->GetCurTextureQuality());
        if (nullptr != curTxQuality)
        {
            return static_cast<int32>(curTxQuality->albedoBaseMipMapLevel);
        }
    }

    return 0;
}
};
