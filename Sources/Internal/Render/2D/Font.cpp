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


#include "Render/2D/Font.h"
#include "Core/Core.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "FontManager.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Utils/StringFormat.h"
#include "Utils/CRC32.h"

namespace DAVA
{
	
	
int32 Font::globalFontDPI = 72;

void Font::SetDPI(int32 dpi)
{
	globalFontDPI = dpi;
}

int32 Font::GetDPI()
{
	return globalFontDPI;
}
	
Font::Font()
:	size(14.0f)
,	verticalSpacing(0)
{
	FontManager::Instance()->RegisterFont(this);
}

Font::~Font()
{
	FontManager::Instance()->UnregisterFont(this);
}


bool Font::IsEqual(const Font *font) const
{
    if(!font)
    {
        return false;
    }
    
	if (fontType != font->fontType) 
	{
		return false;
	}
	if (size != font->size || verticalSpacing != font->verticalSpacing)
	{
		return false;
	}
	
	return true;
}

uint32 Font::GetHashCode()
{
	String rawHashString = GetRawHashString();
	return CRC32::ForBuffer(rawHashString.c_str(), static_cast<uint32>(rawHashString.length()));
}

String Font::GetRawHashString()
{
	return Format("%i_%.0f_%i", fontType, size, verticalSpacing);
}

void Font::SetSize(float32 _size)
{
	size = _size;
}

float32	Font::GetSize() const
{
	return size;
}

void Font::SetVerticalSpacing(int32 _verticalSpacing)
{
	verticalSpacing = _verticalSpacing;
}

int32 Font::GetVerticalSpacing() const
{
	return verticalSpacing;
}

Size2i Font::GetStringSize(const WideString &str, Vector<float32> *charSizes)
{
	StringMetrics metrics = GetStringMetrics(str, charSizes);
	return Size2i(metrics.width, metrics.height);
}

Font::eFontType Font::GetFontType() const
{
    return  fontType;
}

YamlNode * Font::SaveToYamlNode() const
{
    YamlNode *node = new YamlNode(YamlNode::TYPE_MAP);
    
    VariantType *nodeValue = new VariantType();
    //Type
    node->Set("type", "Font");
    //Font size
    node->Set("size", this->GetSize());
    //Vertical Spacing
    node->Set("verticalSpacing", this->GetVerticalSpacing());

    //Ascend / descend
    node->Set("ascendScale", this->GetAscendScale());
    node->Set("descendScale", this->GetDescendScale());

    SafeDelete(nodeValue);
    
    return node;
}

void Font::SetAscendScale(float32 ascendScale)
{
    // Not implemented
}

DAVA::float32 Font::GetAscendScale() const
{
    return 1.f;
}

void Font::SetDescendScale(float32 descendScale)
{
    // Not implemented
}

DAVA::float32 Font::GetDescendScale() const
{
    return 1.f;
}

};