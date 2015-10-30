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


#ifndef __DAVAENGINE_GRAPHICFONT_H__
#define __DAVAENGINE_GRAPHICFONT_H__

#include "Render/2D/Font.h"
#include "Render/Renderer.h"

namespace DAVA
{
	
#define GRAPHIC_FONT_CACHE_SIZE 1000 //text cache size
#define GRAPHIC_FONT_INDEX_BUFFER_SIZE ((GRAPHIC_FONT_CACHE_SIZE)*6)

class GraphicInternalFont;

class GraphicFont : public Font
{
public:
    class GraphicFontVertex
    {
    public:
        Vector3 position;
        Vector2 texCoord;
    };

    GraphicFont();

protected:
    virtual ~GraphicFont();

public:
    static GraphicFont* Create(const FilePath& descriptorPath, const FilePath& texturePath);

    /**
     \brief Get string size(rect).
     \param[in] str - processed string
     \param[in, out] charSizes - if present(not NULL), will contain widths of every symbol in str
     \returns bounding rect for string in pixels
     */
    virtual Font::StringMetrics GetStringMetrics(const WideString& str, Vector<float32>* charSizes = 0) const;

    /**
     \brief Checks if symbol is present in font.
     \param[in] ch - tested symbol
     \returns true if symbol is available, false otherwise
     */
    virtual bool IsCharAvaliable(char16 ch) const;

    /**
     \brief Get height of highest symbol in font.
     \returns height in pixels
     */
    virtual uint32 GetFontHeight() const;

    /**
     \brief Clone font.
     */
    virtual Font* Clone() const;

    /**
     \brief Get font texture
     */
    Texture* GetTexture() const;

    /**
     \brief Tests if two fonts are the same.
     */
    virtual bool IsEqual(const Font* font) const;

    // Put font properties into YamlNode
    virtual YamlNode* SaveToYamlNode() const;

    //We need to return font path
    const FilePath& GetFontPath() const;

    Font::StringMetrics DrawStringToBuffer(const WideString& str,
                                           int32 xOffset,
                                           int32 yOffset,
                                           GraphicFontVertex* vertexBuffer,
                                           int32& charDrawed,
                                           Vector<float32>* charSizes = NULL,
                                           int32 justifyWidth = 0,
                                           int32 spaceAddon = 0) const;

    float32 GetSpread() const;

protected:
    // Get the raw hash string (identical for identical fonts).
    virtual String GetRawHashString();

private:
    float32 GetSizeScale() const;
    bool LoadTexture(const FilePath& path);

    GraphicInternalFont* fontInternal;
    Texture* texture;
};

inline Texture* GraphicFont::GetTexture() const
{
    return texture;
}
}

#endif //__DAVAENGINE_GRAPHICFONT_H__
