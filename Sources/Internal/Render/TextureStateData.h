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

#ifndef __DAVAENGINE_TEXTURESTATEDATA_H__
#define __DAVAENGINE_TEXTURESTATEDATA_H__

#define MAX_TEXTURE_COUNT 8

namespace DAVA
{
class Texture;
class TextureStateData
{
	friend class RenderState;
	
public:
	
	inline TextureStateData();

	inline TextureStateData(const TextureStateData& src);
	
	inline ~TextureStateData();
	
	inline TextureStateData& operator=(const TextureStateData& src);
	
	inline void SetTexture(uint32 index, Texture* tx);
	
	inline Texture* GetTexture(uint32 index);
	
	inline void GetMinMaxIndices(uint32& minIndex, uint32& maxIndex) const;
	
	inline void Clear();
	
	inline bool operator==(const TextureStateData& data) const;
	
private:

	uint32 minmaxTextureIndex;
	Texture* textures[MAX_TEXTURE_COUNT];

private:
	
	inline void ReleaseAll();
	
	inline void RetainAll();
};

inline TextureStateData::TextureStateData()
{
    minmaxTextureIndex = 0;
    memset(textures, 0, sizeof(textures));
}

inline TextureStateData::TextureStateData(const TextureStateData& src)
{
    minmaxTextureIndex = src.minmaxTextureIndex;
    memcpy(textures, src.textures, sizeof(textures));
    
    RetainAll();
}

inline TextureStateData::~TextureStateData()
{
    ReleaseAll();
}

inline TextureStateData& TextureStateData::operator=(const TextureStateData& src)
{
    if(this != &src)
    {
        minmaxTextureIndex = src.minmaxTextureIndex;
        
        for(size_t i = 0; i < MAX_TEXTURE_COUNT; ++i)
        {
            Texture* tmp = textures[i];
            
            textures[i] = SafeRetain(src.textures[i]);
            
            SafeRelease(tmp);
        }
    }
    
    return *this;
}

inline void TextureStateData::SetTexture(uint32 index, Texture* tx)
{
    DVASSERT(index < MAX_TEXTURE_COUNT);
    
    if(textures[index] != tx)
    {
        SafeRelease(textures[index]);
        textures[index] = SafeRetain(tx);
        
        uint32 minIndex = 0;
        uint32 maxIndex = 0;
        GetMinMaxIndices(minIndex, maxIndex);
        
        if(index < minIndex)
        {
            minmaxTextureIndex = ((minmaxTextureIndex & 0xFFFFFF00) | index);
        }
        else if(index > maxIndex)
        {
            minmaxTextureIndex = ((minmaxTextureIndex & 0xFFFF00FF) | (index << 8));
        }
    }
}

inline Texture* TextureStateData::GetTexture(uint32 index)
{
    DVASSERT(index < MAX_TEXTURE_COUNT);
    return textures[index];
}

inline void TextureStateData::GetMinMaxIndices(uint32& minIndex, uint32& maxIndex) const
{
    minIndex = minmaxTextureIndex & 0x000000FF;
    maxIndex = ((minmaxTextureIndex & 0x0000FF00) >> 8);
}

inline void TextureStateData::Clear()
{
    ReleaseAll();
}

inline bool TextureStateData::operator==(const TextureStateData& data) const
{
    bool dataEquals = (minmaxTextureIndex == data.minmaxTextureIndex);
    if(dataEquals)
    {
        uint32 minIndex = 0;
        uint32 maxIndex = 0;
        GetMinMaxIndices(minIndex, maxIndex);
        
        for(uint32 i = minIndex; (i <= maxIndex) && dataEquals; ++i)
        {
            dataEquals = dataEquals && (textures[i] == data.textures[i]);
        }
    }
    
    return dataEquals;
}

inline void TextureStateData::ReleaseAll()
{
    for(size_t i = 0; i < MAX_TEXTURE_COUNT; ++i)
    {
        SafeRelease(textures[i]);
    }
}

inline void TextureStateData::RetainAll()
{
    for(size_t i = 0; i < MAX_TEXTURE_COUNT; ++i)
    {
        SafeRetain(textures[i]);
    }
}

};

#endif
