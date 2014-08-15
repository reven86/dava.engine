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


#include "Render/RenderTarget/RenderTextureDescriptor.h"

namespace DAVA
{

RenderTextureDescriptor::RenderTextureDescriptor() :
textureType(Texture::TEXTURE_TYPE_COUNT),
textureWrapModeS(Texture::WRAP_CLAMP_TO_EDGE),
textureWrapModeT(Texture::WRAP_CLAMP_TO_EDGE),
textureMinFilter(Texture::FILTER_NEAREST),
textureMagFilter(Texture::FILTER_NEAREST)
{

}

RenderTextureDescriptor::RenderTextureDescriptor(Texture::TextureType type,
                                                 Texture::TextureWrap wrapModeS,
                                                 Texture::TextureWrap wrapModeT,
                                                 Texture::TextureFilter minFilter,
                                                 Texture::TextureFilter magFilter) :

textureType(type),
textureWrapModeS(wrapModeS),
textureWrapModeT(wrapModeT),
textureMinFilter(minFilter),
textureMagFilter(magFilter)
{
}


void RenderTextureDescriptor::SetTextureType(Texture::TextureType type)
{
    textureType = type;
}

Texture::TextureType RenderTextureDescriptor::GetTextureType() const
{
    return textureType;
}

void RenderTextureDescriptor::SetWrapModeS(Texture::TextureWrap wrapMode)
{
    textureWrapModeS = wrapMode;
}

Texture::TextureWrap RenderTextureDescriptor::GetWrapModeS() const
{
    return textureWrapModeS;
}

void RenderTextureDescriptor::SetWrapModeT(Texture::TextureWrap wrapMode)
{
    textureWrapModeT = wrapMode;
}

Texture::TextureWrap RenderTextureDescriptor::GetWrapModeT() const
{
    return textureWrapModeT;
}

void RenderTextureDescriptor::SetMinFilter(Texture::TextureFilter texFilter)
{
    textureMinFilter = texFilter;
}

Texture::TextureFilter RenderTextureDescriptor::GetMinFilter() const
{
    return textureMinFilter;
}

void RenderTextureDescriptor::SetMagFilter(Texture::TextureFilter texFilter)
{
    textureMagFilter = texFilter;
}

Texture::TextureFilter RenderTextureDescriptor::GetMagFilter() const
{
    return textureMagFilter;
}

bool RenderTextureDescriptor::IsValid() const
{
    return (textureType != Texture::TEXTURE_TYPE_COUNT);
}

};