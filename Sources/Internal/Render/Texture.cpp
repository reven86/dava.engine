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
#include "Render/RenderManager.h"
#include "Utils/StringFormat.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/File.h"
#include "Render/D3D9Helpers.h"
#include "FileSystem/FileSystem.h"
#include "Render/OGLHelpers.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Render/RenderHelper.h"

#if defined(__DAVAENGINE_IPHONE__) 
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#elif defined(__DAVAENGINE_MACOS__)
#include <ApplicationServices/ApplicationServices.h>
#endif //PLATFORMS

#include "Render/Image/ImageSystem.h"
#include "Render/Image/ImageConvert.h"
#include "Render/OGLHelpers.h"

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
    
#if defined __DAVAENGINE_OPENGL__
static GLuint CUBE_FACE_GL_NAMES[] =
{
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};
	
#define SELECT_GL_TEXTURE_TYPE(__engineTextureType__) ((Texture::TEXTURE_CUBE == __engineTextureType__) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D)
#endif //#if defined __DAVAENGINE_OPENGL__
	
static int32 CUBE_FACE_MAPPING[] =
{
	Texture::CUBE_FACE_POSITIVE_X,
	Texture::CUBE_FACE_NEGATIVE_X,
	Texture::CUBE_FACE_POSITIVE_Y,
	Texture::CUBE_FACE_NEGATIVE_Y,
	Texture::CUBE_FACE_POSITIVE_Z,
	Texture::CUBE_FACE_NEGATIVE_Z
};

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

	Texture * texture = NULL;
	TexturesMap::iterator it = textureMap.find(FILEPATH_MAP_KEY(pathName));
	if (it != textureMap.end())
	{
		texture = it->second;
		texture->Retain();

		return texture;
	}

    return 0;
}

void Texture::AddToMap(Texture *tex)
{
    if(!tex->texDescriptor->pathname.IsEmpty())
    {
        textureMapMutex.Lock();
        DVASSERT(textureMap.find(FILEPATH_MAP_KEY(tex->texDescriptor->pathname)) == textureMap.end());
		textureMap[FILEPATH_MAP_KEY(tex->texDescriptor->pathname)] = tex;
        textureMapMutex.Unlock();
    }
}

    
Texture::Texture()
    : id(0)
    , width(0)
    , height(0)
    , loadedAsFile(GPU_ORIGIN)
    , state(STATE_INVALID)
    , textureType(Texture::TEXTURE_2D)
    , depthFormat(DEPTH_NONE)
    , isRenderTarget(false)
    , isPink(false)
    , invalidater(NULL)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

#ifdef __DAVAENGINE_DIRECTX9__
	saveTexture = 0;
	renderTargetModified = false;
    renderTargetAutosave = true;
#endif //#ifdef __DAVAENGINE_DIRECTX9__


#ifdef __DAVAENGINE_OPENGL__
	fboID = -1;
	rboID = -1;
#if defined(__DAVAENGINE_ANDROID__)
    stencilRboID = -1;
#endif

#endif

    texDescriptor = new TextureDescriptor;
}

Texture::~Texture()
{
    if(invalidater)
    {
        invalidater->RemoveTexture(this);
        invalidater = NULL;
    }
    ReleaseTextureData();
	SafeDelete(texDescriptor);
}
    
void Texture::ReleaseTextureData()
{
#if defined(__DAVAENGINE_ANDROID__)
	uint32 platformStencilRboID = stencilRboID;
#else
	uint32 platformStencilRboID = rboID;
#endif

	state = STATE_INVALID;
    uint32 tt = textureType;

    Function<void()> fn = Bind(&Texture::ReleaseTextureDataInternal, this, tt, id, fboID, rboID, platformStencilRboID);
	JobManager::Instance()->CreateMainJob(fn);

    id = 0;
	fboID = -1;
	rboID = -1;
#if defined(__DAVAENGINE_ANDROID__)
    stencilRboID = -1;
#endif
    isRenderTarget = false;
}

void Texture::ReleaseTextureDataInternal(uint32 textureType, uint32 textureID, uint32 fboID, uint32 rboID, uint32 stencilRboID)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

#if defined(__DAVAENGINE_OPENGL__)
	//if(RenderManager::Instance()->GetTexture() == this)
	//{//to avoid drawing deleted textures
	//	RenderManager::Instance()->SetTexture(0);
	//}

	//VI: reset texture for the current texture type in order to avoid
	//issue when cubemap texture was deleted while being binded to the state
    if(RenderManager::Instance()->HWglGetLastTextureID(textureType) == static_cast<int32>(textureID))
	{
		RenderManager::Instance()->HWglBindTexture(0, textureType);
	}
    
	if(fboID != (uint32)-1)
	{
		RENDER_VERIFY(glDeleteFramebuffers(1, &fboID));
	}
    
#if defined(__DAVAENGINE_ANDROID__)
    if (stencilRboID != (uint32)-1)
    {
        RENDER_VERIFY(glDeleteRenderbuffers(1, &stencilRboID));
    }
#endif

	if (rboID != (uint32)-1)
	{
		RENDER_VERIFY(glDeleteRenderbuffers(1, &rboID));
	}

    if(textureID)
	{
        RENDER_VERIFY(glDeleteTextures(1, &textureID));
        DAVA_MEMORY_PROFILER_GPU_DEALLOC(textureID, ALLOC_GPU_TEXTURE);
	}
	
#elif defined(__DAVAENGINE_DIRECTX9__)
	D3DSafeRelease(id);
	D3DSafeRelease(saveTexture);
#endif //#if defined(__DAVAENGINE_OPENGL__)
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

#if defined(__DAVAENGINE_OPENGL__)

	int32 saveId = RenderManager::Instance()->HWglGetLastTextureID(textureType);
	
	RenderManager::Instance()->HWglBindTexture(id, textureType);

    RENDER_VERIFY(glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ));

	const PixelFormatDescriptor & formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(texDescriptor->format);
    if(FORMAT_INVALID != formatDescriptor.formatID)
    {
		GLuint textureMode = GL_TEXTURE_2D;
		
		if(cubeFaceId != Texture::CUBE_FACE_INVALID)
		{
			textureMode = CUBE_FACE_GL_NAMES[cubeFaceId];
		}
		
		if (formatDescriptor.isCompressedFormat)
        {
			RENDER_VERIFY(glCompressedTexImage2D(textureMode, level, formatDescriptor.internalformat, width, height, 0, dataSize, _data));
        }
        else
        {
            RENDER_VERIFY(glTexImage2D(textureMode, level, formatDescriptor.internalformat, width, height, 0, formatDescriptor.format, formatDescriptor.type, _data));
        }
        DAVA_MEMORY_PROFILER_GPU_ALLOC(id, dataSize, ALLOC_GPU_TEXTURE);
    }
	
	if(0 != saveId)
	{
		RenderManager::Instance()->HWglBindTexture(saveId, textureType);
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
	int32 pixelSizeInBits = PixelFormatDescriptor::GetPixelFormatSize(format);
	if (format ==  FORMAT_RGBA8888)
	{
		//int32 pitchInBytes = 

		uint8 * destBits = (uint8*)rect.pBits;
		uint8 * sourceBits = (uint8*)_data;
		for (uint32 h = 0; h < height * width; ++h)
		{
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
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	Image *image = Image::CreateFromData(_width, _height, _format, _data);
	if(!image) return NULL;

	Texture * texture = new Texture();
	texture->texDescriptor->Initialize(WRAP_CLAMP_TO_EDGE, generateMipMaps);
    
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
	texture->texDescriptor->Initialize(WRAP_CLAMP_TO_EDGE, generateMipMaps);
    
    Vector<Image *> *images = new Vector<Image *>();
    image->Retain();
    images->push_back(image);
	
    texture->SetParamsFromImages(images);
	texture->FlushDataToRenderer(images);
    
	return texture;
}

	
void Texture::SetWrapMode(TextureWrap wrapS, TextureWrap wrapT)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

#if defined(__DAVAENGINE_OPENGL__)
	int32 saveId = RenderManager::Instance()->HWglGetLastTextureID(textureType);
	
	RenderManager::Instance()->HWglBindTexture(id, textureType);
	
	RENDER_VERIFY(glTexParameteri(SELECT_GL_TEXTURE_TYPE(textureType), GL_TEXTURE_WRAP_S, TEXTURE_WRAP_MAP[wrapS]));
	RENDER_VERIFY(glTexParameteri(SELECT_GL_TEXTURE_TYPE(textureType), GL_TEXTURE_WRAP_T, TEXTURE_WRAP_MAP[wrapT]));

	if (saveId != 0)
	{
		RenderManager::Instance()->HWglBindTexture(saveId, textureType);
	}
#elif defined(__DAVAENGINE_DIRECTX9____)
	
#endif //#if defined(__DAVAENGINE_OPENGL__) 
}

void Texture::SetMinMagFilter(TextureFilter minFilter, TextureFilter magFilter)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

#if defined(__DAVAENGINE_OPENGL__)
	int32 saveId = RenderManager::Instance()->HWglGetLastTextureID(textureType);

	RenderManager::Instance()->HWglBindTexture(id, textureType);

	RENDER_VERIFY(glTexParameteri(SELECT_GL_TEXTURE_TYPE(textureType), GL_TEXTURE_MIN_FILTER, TEXTURE_FILTER_MAP[minFilter]));
	RENDER_VERIFY(glTexParameteri(SELECT_GL_TEXTURE_TYPE(textureType), GL_TEXTURE_MAG_FILTER, TEXTURE_FILTER_MAP[magFilter]));

	if (saveId != 0)
	{
		RenderManager::Instance()->HWglBindTexture(saveId, textureType);
	}
#elif defined(__DAVAENGINE_DIRECTX9____)
	
#endif //#if defined(__DAVAENGINE_OPENGL__) 
}
	
void Texture::GenerateMipmaps()
{
	uint32 jobId = JobManager::Instance()->CreateMainJob(MakeFunction(this, &Texture::GenerateMipmapsInternal));
	JobManager::Instance()->WaitMainJobID(jobId);
}

void Texture::GenerateMipmapsInternal()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	const PixelFormatDescriptor & formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(texDescriptor->format);
	if(formatDescriptor.isCompressedFormat)
    {
		return;
	}
    
    
#if defined(__DAVAENGINE_OPENGL__)
    
	int32 saveId = RenderManager::Instance()->HWglGetLastTextureID(textureType);
	
	RenderManager::Instance()->HWglBindTexture(id, textureType);
		
    Image * image0 = CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);
    Vector<Image *> images = image0->CreateMipMapsImages(texDescriptor->dataSettings.GetIsNormalMap());
    SafeRelease(image0);

    for (size_t i = 1, n = images.size(); i < n; ++i)
    {
        TexImage(images[i]->mipmapLevel != (uint32)-1 ? static_cast<int32>(images[i]->mipmapLevel) : static_cast<int32>(i),
                 images[i]->width, images[i]->height, images[i]->data, images[i]->dataSize, images[i]->cubeFaceID);
    }

    ReleaseImages(&images);

    RENDER_VERIFY(glTexParameteri(SELECT_GL_TEXTURE_TYPE(textureType), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	RENDER_VERIFY(glTexParameteri(SELECT_GL_TEXTURE_TYPE(textureType), GL_TEXTURE_MAG_FILTER, GL_LINEAR));

	if (saveId != 0)
	{
		RenderManager::Instance()->HWglBindTexture(saveId, textureType);
	}
	
	
#elif defined(__DAVAENGINE_DIRECTX9__)

#endif // #if defined(__DAVAENGINE_OPENGL__)
}



void Texture::GeneratePixelesation()
{
	uint32 jobId = JobManager::Instance()->CreateMainJob(MakeFunction(this, &Texture::GeneratePixelesationInternal));
	JobManager::Instance()->WaitMainJobID(jobId);
}

void Texture::GeneratePixelesationInternal()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

#if defined(__DAVAENGINE_OPENGL__)
    
	int saveId = RenderManager::Instance()->HWglGetLastTextureID(textureType);
	
	RenderManager::Instance()->HWglBindTexture(id, textureType);
		
	RENDER_VERIFY(glTexParameteri(SELECT_GL_TEXTURE_TYPE(textureType), GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	RENDER_VERIFY(glTexParameteri(SELECT_GL_TEXTURE_TYPE(textureType), GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    
	if (saveId != 0)
	{
		RenderManager::Instance()->HWglBindTexture(saveId, textureType);
	}
	
	
#elif defined(__DAVAENGINE_DIRECTX9__)
    
#endif // #if defined(__DAVAENGINE_OPENGL__)
}
    

Texture * Texture::CreateFromImage(TextureDescriptor *descriptor, eGPUFamily gpu)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	Texture * texture = new Texture();
	texture->texDescriptor->Initialize(descriptor);

    Vector<Image *> * images = new Vector<Image *> ();
    
	bool loaded = texture->LoadImages(gpu, images);
    if(!loaded)
	{
		Logger::Error("[Texture::CreateFromImage] Cannot load texture from image. Descriptor: %s, GPU: %s",
            descriptor->pathname.GetAbsolutePathname().c_str(), GlobalEnumMap<eGPUFamily>::Instance()->ToString(gpu));

        SafeDelete(images);
		SafeRelease(texture);
		return NULL;
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
	if(texDescriptor->IsCubeMap() && (!GPUFamilyDescriptor::IsGPUForDevice(gpu)))
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
            if(faceImage.size() == 0)
			{
                Logger::Error("[Texture::LoadImages] Cannot open file %s", currentfacePath.GetAbsolutePathname().c_str());

				ReleaseImages(images);
				return false;
			}
            
			DVASSERT(faceImage.size() == 1);

			faceImage[0]->cubeFaceID = CUBE_FACE_MAPPING[i];
			faceImage[0]->mipmapLevel = 0;

            //cubemap formats validation
            if(FORMAT_INVALID == imagesFormat)
            {
                imagesFormat = faceImage[0]->format;
            }
            else if(imagesFormat != faceImage[0]->format)
            {
                Logger::Error("[Texture::LoadImages] Face(%s) has different pixel format(%s)", currentfacePath.GetAbsolutePathname().c_str(), PixelFormatDescriptor::GetPixelFormatString(faceImage[0]->format));
                
                ReleaseImages(images);
                return false;
            }
            //end of cubemap formats validation
            
            if(texDescriptor->GetGenerateMipMaps())
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
        if(images->size() == 1 && gpu == GPU_ORIGIN && texDescriptor->GetGenerateMipMaps())
        {
            Image * img = *images->begin();
            *images = img->CreateMipMapsImages(texDescriptor->dataSettings.GetIsNormalMap());
            SafeRelease(img);
        }
    }

    if (0 == images->size())
    {
        Logger::Error("[Texture::LoadImages] Loaded images count is zero");
        return false;
    }

	bool isSizeCorrect = CheckImageSize(*images);
	if(!isSizeCorrect)
	{
        Logger::Error("[Texture::LoadImages] Size if loaded images is invalid (not power of 2)");

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

	textureType = (img->cubeFaceID != Texture::CUBE_FACE_INVALID) ? Texture::TEXTURE_CUBE : Texture::TEXTURE_2D;
    
    state = STATE_DATA_LOADED;
}

void Texture::FlushDataToRenderer(Vector<Image *> * images)
{
    Function<void()> fn = Bind(MakeFunction(MakeSharedObject(this), &Texture::FlushDataToRendererInternal), images);
	JobManager::Instance()->CreateMainJob(fn);
}

void Texture::FlushDataToRendererInternal(Vector<Image *> * images)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	DVASSERT(images->size() != 0);
	DVASSERT(Thread::IsMainThread());

#if defined(__DAVAENGINE_OPENGL__)
	GenerateID();
#elif defined(__DAVAENGINE_DIRECTX9__)
	id = CreateTextureNative(Vector2((float32)_width, (float32)_height), texture->format, false, 0);
#endif //#if defined(__DAVAENGINE_OPENGL__)

    for (size_t i = 0, n = images->size(); i < n; ++i)
    {
        Image *img = (*images)[i];
        TexImage(img->mipmapLevel != (uint32)-1 ? static_cast<int32>(img->mipmapLevel) : static_cast<int32>(i),
                 img->width, img->height, img->data, img->dataSize, img->cubeFaceID);
    }

#if defined(__DAVAENGINE_OPENGL__)

	int32 saveId = RenderManager::Instance()->HWglGetLastTextureID(textureType);

	RenderManager::Instance()->HWglBindTexture(id, textureType);

	RENDER_VERIFY(glTexParameteri(SELECT_GL_TEXTURE_TYPE(textureType), GL_TEXTURE_WRAP_S, TEXTURE_WRAP_MAP[texDescriptor->drawSettings.wrapModeS]));
	RENDER_VERIFY(glTexParameteri(SELECT_GL_TEXTURE_TYPE(textureType), GL_TEXTURE_WRAP_T, TEXTURE_WRAP_MAP[texDescriptor->drawSettings.wrapModeT]));

    if (pixelizationFlag)
    {
        RENDER_VERIFY(glTexParameteri(SELECT_GL_TEXTURE_TYPE(textureType), GL_TEXTURE_MIN_FILTER, TEXTURE_FILTER_MAP[FILTER_NEAREST]));
        RENDER_VERIFY(glTexParameteri(SELECT_GL_TEXTURE_TYPE(textureType), GL_TEXTURE_MAG_FILTER, TEXTURE_FILTER_MAP[FILTER_NEAREST]));
    }
    else
    {
        RENDER_VERIFY(glTexParameteri(SELECT_GL_TEXTURE_TYPE(textureType), GL_TEXTURE_MIN_FILTER, TEXTURE_FILTER_MAP[texDescriptor->drawSettings.minFilter]));
        RENDER_VERIFY(glTexParameteri(SELECT_GL_TEXTURE_TYPE(textureType), GL_TEXTURE_MAG_FILTER, TEXTURE_FILTER_MAP[texDescriptor->drawSettings.magFilter]));
    }

	RenderManager::Instance()->HWglBindTexture(saveId, textureType);
#elif defined(__DAVAENGINE_DIRECTX9__)

#endif //#if defined(__DAVAENGINE_OPENGL__)

	state = STATE_VALID;

	ReleaseImages(images);
    SafeDelete(images);
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

Texture * Texture::CreateFromFile(const FilePath & pathName, const FastName &group, TextureType typeHint)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	Texture * texture = PureCreate(pathName, group);
	if(!texture)
	{
        TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(pathName);
        if(descriptor)
        {
            texture = CreatePink(descriptor->IsCubeMap() ? DAVA::Texture::TEXTURE_CUBE : typeHint);
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

	if(pathName.IsEmpty() || pathName.GetType() == FilePath::PATH_IN_MEMORY)
		return NULL;

    if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::TEXTURE_LOAD_ENABLED))
        return NULL;

    FilePath descriptorPathname = TextureDescriptor::GetDescriptorPathname(pathName);
    Texture * texture = Texture::Get(descriptorPathname);
	if (texture) return texture;
    
    TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(descriptorPathname);
    if(!descriptor) return NULL;
    
    descriptor->SetQualityGroup(group);

	eGPUFamily gpuForLoading = GetGPUForLoading(defaultGPU, descriptor);
	texture = CreateFromImage(descriptor, gpuForLoading);
	if(texture)
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

    ReleaseTextureData();
    
    Image *image = Image::CreateFromData(_width, _height, format, data);
	if(!image) return;
    
    Vector<Image *> *images = new Vector<Image *>();
    images->push_back(image);
	
    SetParamsFromImages(images);
	FlushDataToRenderer(images);
}
    
void Texture::Reload()
{
    ReloadAs(loadedAsFile);
}
    
void Texture::ReloadAs(eGPUFamily gpuFamily)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(isRenderTarget == false);
    
    ReleaseTextureData();

	bool descriptorReloaded = texDescriptor->Reload();
    
	eGPUFamily gpuForLoading = GetGPUForLoading(gpuFamily, texDescriptor);
    Vector<Image *> *images = new Vector<Image *> ();
    
    bool loaded = false;
    if(descriptorReloaded && RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::TEXTURE_LOAD_ENABLED))
    {
	    loaded = LoadImages(gpuForLoading, images);
    }

	if(loaded)
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
}

    
bool Texture::IsLoadAvailable(const eGPUFamily gpuFamily) const
{
    if(texDescriptor->IsCompressedFile())
    {
        return true;
    }
    
    if(GPUFamilyDescriptor::IsGPUForDevice(gpuFamily) && texDescriptor->compression[gpuFamily].format == FORMAT_INVALID)
    {
        return false;
    }
    
    return true;
}

    
int32 Texture::Release()
{
	if(GetRetainCount() == 1)
	{
        textureMapMutex.Lock();
		textureMap.erase(FILEPATH_MAP_KEY(texDescriptor->pathname));
        textureMapMutex.Unlock();
	}
	return BaseObject::Release();
}
	
Texture * Texture::CreateFBO(uint32 w, uint32 h, PixelFormat format, DepthFormat _depthFormat)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	int32 dx = Max((int32)w, 8);
    EnsurePowerOf2(dx);
    
	int32 dy = Max((int32)h, 8);
    EnsurePowerOf2(dy);
    

#if defined(__DAVAENGINE_OPENGL__)

	Texture *tx = Texture::CreateFromData(format, NULL, dx, dy, false);
	DVASSERT(tx);

	tx->depthFormat = _depthFormat;

	tx->HWglCreateFBOBuffers();	

#elif defined(__DAVAENGINE_DIRECTX9__)

	// TODO: Create FBO
	Texture * tx = new Texture();

	tx->width = dx;
	tx->height = dy;
	tx->format = format;

	RenderManager::Instance()->LockNonMain();
	tx->id = CreateTextureNative(Vector2((float32)tx->width, (float32)tx->height), tx->format, true, 0);
	RenderManager::Instance()->UnlockNonMain();

	tx->state = STATE_VALID;
#endif 


    tx->isRenderTarget = true;
    tx->texDescriptor->pathname = Format("FBO texture %d", textureFboCounter);
	AddToMap(tx);
	
	textureFboCounter++;
	
	return tx;
}

#if defined(__DAVAENGINE_OPENGL__)
void Texture::HWglCreateFBOBuffers()
{
    JobManager::Instance()->CreateMainJob(MakeFunction(MakeSharedObject(this), &Texture::HWglCreateFBOBuffersInternal));
}

void Texture::HWglCreateFBOBuffersInternal()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	GLint saveFBO = RenderManager::Instance()->HWglGetLastFBO();
	GLint saveTexture = RenderManager::Instance()->HWglGetLastTextureID(textureType);

	RenderManager::Instance()->HWglBindTexture(id, textureType);

	RENDER_VERIFY(glGenFramebuffers(1, &fboID));
	RenderManager::Instance()->HWglBindFBO(fboID);
    
	if(DEPTH_RENDERBUFFER == depthFormat)
	{
		RENDER_VERIFY(glGenRenderbuffers(1, &rboID));
		RENDER_VERIFY(glBindRenderbuffer(GL_RENDERBUFFER, rboID));
        
#if defined(__DAVAENGINE_ANDROID__)
        if (RenderManager::Instance()->GetCaps().isGlDepth24Stencil8Supported)
#endif
        {
            RENDER_VERIFY(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height));
        }
#if defined(__DAVAENGINE_ANDROID__)
        else
        {
            if (RenderManager::Instance()->GetCaps().isGlDepthNvNonLinearSupported)
            {
                RENDER_VERIFY(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16_NONLINEAR_NV, width, height));
            }
            else
            {
                RENDER_VERIFY(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height));
            }

            if (!RenderManager::Instance()->GetCaps().isGlDepth24Stencil8Supported)
            {
                RENDER_VERIFY(glGenRenderbuffers(1, &stencilRboID));
                RENDER_VERIFY(glBindRenderbuffer(GL_RENDERBUFFER, stencilRboID));
                RENDER_VERIFY(glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height));
            }
        }
#endif
	}

	RENDER_VERIFY(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, id, 0));

	if(DEPTH_RENDERBUFFER == depthFormat)
	{
		RENDER_VERIFY(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID));
#if defined(__DAVAENGINE_ANDROID__)
        if (RenderManager::Instance()->GetCaps().isGlDepth24Stencil8Supported)
#endif
        {
            RENDER_VERIFY(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboID));
        }
#if defined(__DAVAENGINE_ANDROID__)
        else
        {
            RENDER_VERIFY(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilRboID));
        }
#endif
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		Logger::Error("[Texture::HWglCreateFBOBuffers] glCheckFramebufferStatus: %d", status);
	}

	RenderManager::Instance()->HWglBindFBO(saveFBO);

	if(saveTexture)
	{
		RenderManager::Instance()->HWglBindTexture(saveTexture, textureType);
	}

	state = STATE_VALID;
}

#endif //#if defined(__DAVAENGINE_OPENGL__)


	
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
		Logger::FrameworkDebug("%s with id %d (%dx%d) retainCount: %d debug: %s format: %s", t->texDescriptor->pathname.GetAbsolutePathname().c_str(), t->id, t->width, t->height, 
								t->GetRetainCount(), t->debugInfo.c_str(), PixelFormatDescriptor::GetPixelFormatString(t->texDescriptor->format));
		cnt++;
        
        DVASSERT((0 <= t->texDescriptor->format) && (t->texDescriptor->format < FORMAT_COUNT));
        if(FORMAT_INVALID != t->texDescriptor->format)
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
	
#if defined(__DAVAENGINE_ANDROID__)
	
void Texture::Lost()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    RenderResource::Lost();
    
    ReleaseTextureData();
}

void Texture::Invalidate()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	RenderResource::Invalidate();
	
	DVASSERT(id == 0 && "Texture always invalidated");
	if (id)
	{
		return;
	}

	if (invalidater)
    {
        invalidater->InvalidateTexture(this);
    }
    else
    {
        const FilePath& relativePathname = texDescriptor->GetSourceTexturePathname();
        if (relativePathname.GetType() == FilePath::PATH_IN_FILESYSTEM ||
            relativePathname.GetType() == FilePath::PATH_IN_RESOURCES ||
            relativePathname.GetType() == FilePath::PATH_IN_DOCUMENTS)
        {
            Reload();
        }
        else if (relativePathname.GetType() == FilePath::PATH_IN_MEMORY)
        {
            // Make it pink, to prevent craches
            Logger::Debug("[Texture::Invalidate] - invalidater is null");
            MakePink();
        }
        else if (isPink)
        {
            MakePink();
        }
    }
}
#endif //#if defined(__DAVAENGINE_ANDROID__)

Image * Texture::ReadDataToImage()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    const PixelFormatDescriptor & formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(texDescriptor->format);

    Image *image = Image::Create(width, height, formatDescriptor.formatID);
    uint8 *imageData = image->GetData();
    
#if defined(__DAVAENGINE_OPENGL__)
    
    int32 saveFBO = RenderManager::Instance()->HWglGetLastFBO();
    int32 saveId = RenderManager::Instance()->HWglGetLastTextureID(textureType);

    RenderManager::Instance()->HWglBindFBO(fboID);
	RenderManager::Instance()->HWglBindTexture(id, textureType);
    
    if(FORMAT_INVALID != formatDescriptor.formatID)
    {
		RENDER_VERIFY(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        RENDER_VERIFY(glReadPixels(0, 0, width, height, formatDescriptor.format, formatDescriptor.type, (GLvoid *)imageData));
    }

    RenderManager::Instance()->HWglBindFBO(saveFBO);
    RenderManager::Instance()->HWglBindTexture(saveId, textureType);
    
#endif //#if defined(__DAVAENGINE_OPENGL__)
    
    return image; 
}


Image * Texture::CreateImageFromMemory(UniqueHandle renderState)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    Image *image = NULL;
    if(isRenderTarget)
    {
        image = ReadDataToImage();
    }
    else
    {
        Texture * oldRenderTarget = RenderManager::Instance()->GetRenderTarget();

        Texture *renderTarget = Texture::CreateFBO(width, height, texDescriptor->format, DEPTH_NONE);
        RenderHelper::Instance()->Set2DRenderTarget(renderTarget);
        RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
        RenderHelper::Instance()->DrawTexture(this, renderState);

        RenderManager::Instance()->SetRenderTarget(oldRenderTarget);
        
        image = renderTarget->CreateImageFromMemory(renderState);

        SafeRelease(renderTarget);
    }
        
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

Texture * Texture::CreatePink(TextureType requestedType, bool checkers)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	//we need instances for pink textures for ResourceEditor. We use it for reloading for different GPUs
	//pink textures at game is invalid situation
	Texture *tex = new Texture();
	if(Texture::TEXTURE_CUBE == requestedType)
	{
		tex->texDescriptor->Initialize(WRAP_CLAMP_TO_EDGE, true);
		tex->texDescriptor->dataSettings.cubefaceFlags = 0x000000FF;
	}
	else
	{
		tex->texDescriptor->Initialize(WRAP_CLAMP_TO_EDGE, false);
	}

	tex->MakePink(checkers);

	return tex;
}

void Texture::MakePink(bool checkers)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    Vector<Image *> *images = new Vector<Image *> ();
	if(texDescriptor->IsCubeMap())
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

	GeneratePixelesation();
}
    
bool Texture::IsPinkPlaceholder()
{
	return isPink;
}

void Texture::GenerateID()
{
#if defined(__DAVAENGINE_OPENGL__)
	RENDER_VERIFY(glGenTextures(1, &id));
	DVASSERT(id);
#endif //#if defined(__DAVAENGINE_OPENGL__)

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
    if(descriptor->IsCompressedFile())
        return (eGPUFamily)descriptor->exportedAsGpuFamily;
    
    return requestedGPU;
}

void Texture::SetInvalidater(TextureInvalidater* invalidater)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if(this->invalidater)
    {
        this->invalidater->RemoveTexture(this);
    }
	this->invalidater = invalidater;
    if(invalidater != NULL)
    {
        invalidater->AddTexture(this);
    }
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
}


int32 Texture::GetBaseMipMap() const
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if(texDescriptor->GetQualityGroup().IsValid())
    {
        const TextureQuality *curTxQuality = QualitySettingsSystem::Instance()->GetTxQuality(QualitySettingsSystem::Instance()->GetCurTextureQuality());
        if(NULL != curTxQuality)
        {
            return static_cast<int32>(curTxQuality->albedoBaseMipMapLevel);
        }
    }

    return 0;
}

};
