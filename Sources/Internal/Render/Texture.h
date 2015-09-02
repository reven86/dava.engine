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


#ifndef __DAVAENGINE_TEXTURE_H__
#define __DAVAENGINE_TEXTURE_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Render/RenderResource.h"
#include "FileSystem/FilePath.h"
#include "Render/RHI/rhi_Public.h"

#include "Render/UniqueStateSet.h"

#include "MemoryManager/MemoryProfiler.h"

namespace DAVA
{
/**
	\ingroup render
	\brief Class that represents texture objects in our SDK. 
	This class support the following formats: RGBA8888, RGB565, RGBA4444, A8 on all platforms. 
	For iOS it also support compressed PVR formats. (PVR2 and PVR4)
 */
class Image;
class TextureDescriptor;
class File;
class Texture;
	
class TextureInvalidater
{
public:
    virtual ~TextureInvalidater() {};
	virtual void InvalidateTexture(Texture * texture) = 0;
    virtual void AddTexture(Texture * texture) = 0;
    virtual void RemoveTexture(Texture * texture) = 0;
};
	
#ifdef USE_FILEPATH_IN_MAP
    using TexturesMap = Map<FilePath, Texture *>;
#else //#ifdef USE_FILEPATH_IN_MAP
    using TexturesMap = Map<String, Texture *>;
#endif //#ifdef USE_FILEPATH_IN_MAP


class Texture : public RenderResource
{
    DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(ALLOC_POOL_TEXTURE)
public:       
        


	enum TextureState
	{
		STATE_INVALID	=	0,
		STATE_DATA_LOADED,
		STATE_VALID
	};

    const static uint32 INVALID_CUBEMAP_FACE = -1;
    const static uint32 CUBE_FACE_COUNT = 6;
	
    static Array<String, CUBE_FACE_COUNT> FACE_NAME_SUFFIX;

	// Main constructors
    /**
        \brief Create texture from data arrray
        This function creates texture from given format, data pointer and width + height
     
        \param[in] format desired pixel format
        \param[in] data desired data 
        \param[in] width width of new texture
        \param[in] height height of new texture
        \param[in] generateMipMaps generate mipmaps or not
     */
	static Texture * CreateFromData(PixelFormat format, const uint8 *data, uint32 width, uint32 height, bool generateMipMaps);

    /**
        \brief Create texture from data arrray stored at Image
        This function creates texture from given image
     
        \param[in] image stores data
        \param[in] generateMipMaps generate mipmaps or not
     */
	static Texture * CreateFromData(Image *img, bool generateMipMaps);

    /**
        \brief Create text texture from data arrray
        This function creates texture from given format, data pointer and width + height, but adds addInfo string to relativePathname variable for easy identification of textures
        
        \param[in] format desired pixel format
        \param[in] data desired data 
        \param[in] width width of new texture
        \param[in] height height of new texture
        \param[in] addInfo additional info
     */
	static Texture * CreateTextFromData(PixelFormat format, uint8 * data, uint32 width, uint32 height, bool generateMipMaps, const char * addInfo = 0);
    
	/**
        \brief Create texture from given file. Supported formats .png, .pvr (only on iOS). 
		If file cannot be opened, returns "pink placeholder" texture.
        \param[in] pathName path to the png or pvr file
     */
    static Texture * CreateFromFile(const FilePath & pathName, const FastName &group = FastName(), rhi::TextureType typeHint = rhi::TEXTURE_TYPE_2D);

	/**
        \brief Create texture from given file. Supported formats .png, .pvr (only on iOS). 
		If file cannot be opened, returns 0
        \param[in] pathName path to the png or pvr file
     */
	static Texture * PureCreate(const FilePath & pathName, const FastName &group = FastName());    	    
	
    static Texture * CreatePink(rhi::TextureType requestedType = rhi::TEXTURE_TYPE_2D, bool checkers = true);

    static Texture * CreateFBO(uint32 width, uint32 height, PixelFormat format, rhi::TextureType requestedType = rhi::TEXTURE_TYPE_2D);

    
    /**
        \brief Get texture from cache.
        If texture isn't in cache, returns 0
        \param[in] name path of TextureDescriptor
     */
    static Texture * Get(const FilePath & name);


	virtual int32 Release();

	static void	DumpTextures();

	inline int32 GetWidth() const { return width; }
	inline int32 GetHeight() const { return height; }
	
	void GenerateMipmaps();	
	
	void TexImage(int32 level, uint32 width, uint32 height, const void * _data, uint32 dataSize, uint32 cubeFaceId);
    
    void SetWrapMode(rhi::TextureAddrMode wrapU, rhi::TextureAddrMode wrapV, rhi::TextureAddrMode wrapW = rhi::TEXADDR_WRAP);
    void SetMinMagFilter(rhi::TextureFilter minFilter, rhi::TextureFilter magFilter, rhi::TextureMipFilter mipFilter);
        
    /**
        \brief Function to receive pathname of texture object
        \returns pathname of texture
     */
    const FilePath & GetPathname() const;
    void SetPathname(const FilePath& path);
    
    Image * CreateImageFromMemory();

	bool IsPinkPlaceholder();

    void Reload();
    void ReloadAs(eGPUFamily gpuFamily);
	void SetInvalidater(TextureInvalidater* invalidater);
    void ReloadFromData(PixelFormat format, uint8 * data, uint32 width, uint32 height);

	inline TextureState GetState() const;


	void RestoreRenderResource();

    
    void SetDebugInfo(const String & _debugInfo);
    
	static const TexturesMap & GetTextureMap();
    
    uint32 GetDataSize() const;

    static void SetDefaultGPU(eGPUFamily gpuFamily);
    static eGPUFamily GetDefaultGPU();
    
    
    inline const eGPUFamily GetSourceFileGPUFamily() const;
    inline TextureDescriptor * GetDescriptor() const;

	PixelFormat GetFormat() const;

    static void SetPixelization(bool value);
    
    int32 GetBaseMipMap() const;

protected:
    
    void ReleaseTextureData();

	static void AddToMap(Texture *tex);
    
	static Texture * CreateFromImage(TextureDescriptor *descriptor, eGPUFamily gpu);
    
	bool LoadImages(eGPUFamily gpu, Vector<Image *> * images);
    
	void SetParamsFromImages(const Vector<Image *> * images);
	
	void FlushDataToRenderer(Vector<Image *> * images);

	void ReleaseImages(Vector<Image *> * images);
    
    void MakePink(bool checkers = true);	
    	
	void GenerateMipmapsInternal();

    static bool CheckImageSize(const Vector<Image *> &imageSet);
    
	Texture();
	virtual ~Texture();
    
    bool IsLoadAvailable(const eGPUFamily gpuFamily) const;

	static eGPUFamily GetGPUForLoading(const eGPUFamily requestedGPU, const TextureDescriptor *descriptor);

public:							// properties for fast access


    rhi::HTexture handle;
    rhi::SamplerState::Descriptor::Sampler samplerState;

	
    uint32		width:16;			// texture width
	uint32		height:16;			// texture height

    eGPUFamily loadedAsFile:4;
	TextureState state:2;
	uint32		textureType:2;	
	bool		isRenderTarget:1;
	bool		isPink:1;

    FastName		debugInfo;
	TextureInvalidater* invalidater;
    TextureDescriptor *texDescriptor;

    static Mutex textureMapMutex;

    static TexturesMap textureMap;
    static eGPUFamily defaultGPU;
    
    static bool pixelizationFlag;
};
    
// Implementation of inline functions

    
inline const eGPUFamily Texture::GetSourceFileGPUFamily() const
{
    return loadedAsFile;
}

inline Texture::TextureState Texture::GetState() const
{
	return state;
}

inline TextureDescriptor * Texture::GetDescriptor() const
{
    return texDescriptor;
}


};

#endif // __DAVAENGINE_TEXTUREGLES_H__
