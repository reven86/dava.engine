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


#include "UI/UIYamlLoader.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlEmitter.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/FileSystem.h"
#include "Render/2D/GraphicFont.h"
#include "Render/2D/FontManager.h"
#include "Render/2D/TextBlock.h"
#include "Render/2D/FTFont.h"
#include "Utils/Utils.h"

namespace DAVA
{
Font* UIYamlLoader::GetFontByName(const String& fontName) const
{
    return FontManager::Instance()->GetFont(fontName);
}

void UIYamlLoader::LoadFonts(const FilePath& yamlPathname)
{
    ScopedPtr<UIYamlLoader> loader(new UIYamlLoader());
    YamlNode* rootNode = loader->CreateRootNode(yamlPathname);
    if (!rootNode)
    {
        // Empty YAML file.
        Logger::Warning("yaml file: %s is empty", yamlPathname.GetAbsolutePathname().c_str());
        return;
    }
    loader->LoadFontsFromNode(rootNode);
    SafeRelease(rootNode);
}

bool UIYamlLoader::SaveFonts(const FilePath& yamlPathname)
{
    const auto& fontMap = FontManager::Instance()->GetFontMap();
    ScopedPtr<YamlNode> fontsNode(new YamlNode(YamlNode::TYPE_MAP));
    for (const auto& pair : fontMap)
    {
        Font* font = pair.second;
        if (nullptr == font)
            continue;
        fontsNode->AddNodeToMap(pair.first, font->SaveToYamlNode());
    }
    return YamlEmitter::SaveToYamlFile(yamlPathname, fontsNode, File::CREATE | File::WRITE);
}

YamlNode* UIYamlLoader::CreateRootNode(const FilePath& yamlPathname)
{
    YamlParser* parser = YamlParser::Create(yamlPathname);
    if (!parser)
    {
        Logger::Error("Failed to open yaml file: %s", yamlPathname.GetAbsolutePathname().c_str());
        return NULL;
    }
    YamlNode* rootNode = SafeRetain(parser->GetRootNode());
    SafeRelease(parser);
    return rootNode;
}

void UIYamlLoader::LoadFontsFromNode(const YamlNode* rootNode)
{
    for (MultiMap<String, YamlNode*>::const_iterator t = rootNode->AsMap().begin(); t != rootNode->AsMap().end(); ++t)
    {
        YamlNode* node = t->second;

        Font* font = CreateFontFromYamlNode(node);

        if (font)
        {
            FontManager::Instance()->SetFontName(font, t->first);
            SafeRelease(font);
        }
    }
}

Font* UIYamlLoader::CreateFontFromYamlNode(const YamlNode* node)
{
    const YamlNode* typeNode = node->Get("type");
    if (!typeNode)
        return nullptr;

    const String& type = typeNode->AsString();
    Font* font = nullptr;

    if (type == "FTFont")
    {
        const YamlNode* fontNameNode = node->Get("name");
        if (!fontNameNode)
            return nullptr;

        font = FTFont::Create(fontNameNode->AsString());
    }
    else if (type == "GraphicFont")
    {
        const YamlNode* fontNameNode = node->Get("name");
        if (!fontNameNode)
        {
            return nullptr;
        }

        const YamlNode* texNameNode = node->Get("texture");
        if (!fontNameNode)
        {
            return nullptr;
        }

        font = GraphicFont::Create(fontNameNode->AsString(), texNameNode->AsString());
    }

    if (font == nullptr)
    {
        return nullptr;
    }

    float32 fontSize = 10.0f;
    const YamlNode* fontSizeNode = node->Get("size");
    if (fontSizeNode)
        fontSize = fontSizeNode->AsFloat();

    font->SetSize(fontSize);

    const YamlNode* fontVerticalSpacingNode = node->Get("verticalSpacing");
    if (fontVerticalSpacingNode)
    {
        font->SetVerticalSpacing(fontVerticalSpacingNode->AsInt32());
    }

    const YamlNode* fontFontAscendNode = node->Get("ascendScale");
    if (fontFontAscendNode)
    {
        font->SetAscendScale(fontFontAscendNode->AsFloat());
    }

    const YamlNode* fontFontDescendNode = node->Get("descendScale");
    if (fontFontDescendNode)
    {
        font->SetDescendScale(fontFontDescendNode->AsFloat());
    }

    return font;
}
}