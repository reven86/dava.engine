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


#include "Render/MipMapReplacer.h"
#include "Render/Texture.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/Image.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/PixelFormatDescriptor.h"

#define DUMMY_TEXTURES_DIR "~res:/TexDum/"

namespace DAVA
{

void MipMapReplacer::ReplaceMipMaps(Entity * node, int32 level)
{
    Set<Texture *> textures;
    EnumerateTexturesRecursive(node, textures);
    
    Set<Texture *>::iterator endIt = textures.end();
    for(Set<Texture *>::iterator it = textures.begin(); it != endIt; ++it)
        ReplaceMipMap((*it), level);
}

void MipMapReplacer::EnumerateTexturesRecursive(Entity * entity, Set<Texture *> & textures)
{
    if(!entity)
        return;

    int32 childrenCount = entity->GetChildrenCount();
    for(int32 i = 0; i < childrenCount; i++)
        EnumerateTexturesRecursive(entity->GetChild(i), textures);

    RenderObject * ro = GetRenderObject(entity);
    if(ro)
    {
        int32 rbCount = ro->GetRenderBatchCount();
        for(int32 i = 0; i < rbCount; i++)
        {
            RenderBatch * rb = ro->GetRenderBatch(i);
            if(rb)
            {
                DVASSERT(0 && "Vitaliy Borodovsky: Temporarly disabled. Need to rewrite for new materials");
                
                NMaterial * material = rb->GetMaterial();
                if(material)
                {
                    Texture * texture = material->GetTexture(NMaterialTextureName::TEXTURE_ALBEDO);
                    if(texture)
                        textures.insert(texture);
                }
            }
        }
    }
}

void MipMapReplacer::ReplaceMipMap(Texture * texture, int32 level)
{
    if(!texture)
        return;

    if(texture->width != texture->height)
        return;

    FilePath textureFilePath = GetDummyTextureFilePath(texture);
    if(!textureFilePath.IsEmpty())
    {
        Vector<Image*> mipImg;
        ImageSystem::Instance()->Load(textureFilePath, mipImg);
        if(mipImg.size())
        {
            uint32 mipMapSize = texture->width / (1 << level);

            int32 imgCount = static_cast<int32>(mipImg.size());
            for(int i = 0; i < imgCount; i++)
            {
                Image * dummyImg = mipImg[i];
                if(dummyImg->width == mipMapSize)
                {
                    texture->TexImage(level, dummyImg->width, dummyImg->height, dummyImg->data, dummyImg->dataSize, Texture::CUBE_FACE_INVALID);
                }

                SafeRelease(dummyImg);
            }
        }
    }
    else
    {
        return ReplaceMipMapFromMemory(texture, level);
    }
}

void MipMapReplacer::ReplaceMipMapFromMemory(Texture * texture, int32 level)
{
    uint32 mipMapSize = texture->width / (1 << level);
    if(mipMapSize < 2)  //don't replace mipmaps less than 2x2
        return;

    uint32 dataSize = 0;
    uint8 * data = 0;

	PixelFormat format = texture->GetFormat();
    if(format == FORMAT_RGB888)
    {
        uint32 pixelsCount = mipMapSize * mipMapSize;
        dataSize = pixelsCount * 3;
        data = new uint8[dataSize];

        for(uint32 i = 0; i < pixelsCount; i++)
        {
            data[i * 3] = 0xff;
            data[i * 3 + 1] = 0;
            data[i * 3 + 2] = 0;
        }
    }
    else
    {
        int32 elementBytesCount = PixelFormatDescriptor::GetPixelFormatSizeInBytes(format);
        uint32 elementValue = GetReplaceValue(format);

        uint32 pixelsCount = mipMapSize * mipMapSize;
        dataSize = pixelsCount * elementBytesCount;
        data = new uint8[dataSize];

        uint32 * dataPt = (uint32 *)data;
        uint32 intCount = dataSize / 4;

        while(intCount)
        {
            *dataPt++ = elementValue;
            intCount--;
        }
    }

    texture->TexImage(level, mipMapSize, mipMapSize, data, dataSize, Texture::CUBE_FACE_INVALID);

    SafeDeleteArray(data);
}

uint32 MipMapReplacer::GetReplaceValue(PixelFormat format)
{
    uint32 elementValue = 0;
    switch (format)
    {
    case FORMAT_RGB565:
        elementValue = 0xf800f800;//0xf800 - 5bits of red
        break;
    case FORMAT_RGBA4444:
        elementValue = 0xf00ff00f;//0xf00f - 4bits of red and 4bits of alpha
        break;
    case FORMAT_RGBA5551:
        elementValue = 0xf801f801;//0xf801 - 5bits of red and 1bit of alpha
        break;
    case FORMAT_RGBA8888:
        elementValue = 0xff0000ff;//0xff0000ff - 8bits of red and 8bits of alpha
        break;
    case FORMAT_A8:
        elementValue = 0xffffffff;//0xff - 8bits of alpha
        break;
    default:
        break;
    }
    return elementValue;
}

FilePath MipMapReplacer::GetDummyTextureFilePath(Texture * texture)
{
    String formatFile;
    switch (texture->GetFormat())
    {
    case FORMAT_ATC_RGB:
        formatFile = "atc.dds";
        break;
    case FORMAT_ATC_RGBA_EXPLICIT_ALPHA:
        formatFile = "atce.dds";
        break;
    case FORMAT_ATC_RGBA_INTERPOLATED_ALPHA:
        formatFile = "atci.dds";
        break;
    case FORMAT_DXT1:
        formatFile = "dxt1.dds";
        break;
    case FORMAT_DXT1A:
        formatFile = "dxt1a.dds";
        break;
    case FORMAT_DXT3:
        formatFile = "dxt3.dds";
        break;
    case FORMAT_DXT5:
        formatFile = "dxt5.dds";
        break;
    case FORMAT_DXT5NM:
        formatFile = "dxt5nm.dds";
        break;
    case FORMAT_ETC1:
        formatFile = "etc1.pvr";
        break;
    case FORMAT_PVR2:
        formatFile = "pvr2.pvr";
        break;
    case FORMAT_PVR4:
        formatFile = "pvr4.pvr";
        break;
    default:
        return FilePath();
    }

    return FilePath(DUMMY_TEXTURES_DIR + formatFile);
}

};
