#include "Render/2D/Font.h"
#include "Core/Core.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "FontManager.h"
#include "UI/UIControlSystem.h"
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
    : size(14.0f)
    , verticalSpacing(0)
{
    FontManager::Instance()->RegisterFont(this);
}

Font::~Font()
{
    FontManager::Instance()->UnregisterFont(this);
}

bool Font::IsEqual(const Font* font) const
{
    if (!font)
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

float32 Font::GetSize() const
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

Size2i Font::GetStringSize(const WideString& str, Vector<float32>* charSizes)
{
    StringMetrics metrics = GetStringMetrics(str, charSizes);
    return Size2i(int32(std::ceil(metrics.width)), int32(std::ceil(metrics.height)));
}

Font::eFontType Font::GetFontType() const
{
    return fontType;
}

YamlNode* Font::SaveToYamlNode() const
{
    YamlNode* node = new YamlNode(YamlNode::TYPE_MAP);

    VariantType* nodeValue = new VariantType();
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