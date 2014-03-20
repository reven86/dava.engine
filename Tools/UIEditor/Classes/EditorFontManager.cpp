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



#include "EditorFontManager.h"
#include "StringUtils.h"

static const String DEFAULT_FONT_NAME = "MyriadPro-Regular.otf";
static const String DEFAULT_FONT_PATH = "~res:/Fonts/MyriadPro-Regular.otf";

EditorFontManager::EditorFontManager()
{
	defaultFont = NULL;
	baseFont = NULL;
	Init();
}

EditorFontManager::~EditorFontManager()
{
	Reset();
}

void EditorFontManager::Init()
{
	baseFont = LoadFont(DEFAULT_FONT_PATH, DEFAULT_FONT_NAME);
}

void EditorFontManager::Reset()
{
	defaultFont = NULL;
	baseFont = NULL;
	
	for (FONTSMAP::iterator iter = fonts.begin(); iter != fonts.end(); ++iter)
	{
		Font* font = iter->second;
		SafeRelease(font);
	}
}

Font* EditorFontManager::LoadFont(const String& fontPath, const String& fontName)
{
	Font* font = GetFont(fontName);
	if (font)
		return font;
	
    static const float defaultFontSize = 12.0f;
	font = FTFont::Create(fontPath);
	if (font)
	{
		font->SetSize(defaultFontSize);
		
		fonts[fontName] = font;
        //If font was successfully loaded - emit the signal 
        emit FontLoaded();
	}
	
	return font;
}

Font* EditorFontManager::GetDefaultFont() const
{
	return defaultFont ? defaultFont : baseFont;
}

void EditorFontManager::SetDefaultFont(Font *font)
{
	defaultFont = font->Clone();
}

void EditorFontManager::ResetDefaultFont()
{
	defaultFont = NULL;
}

Font* EditorFontManager::GetFont(const String& name) const
{
	FONTSMAP::const_iterator iter = fonts.find(name);
	if (iter != fonts.end())
	{
		return iter->second;
	}
	return NULL;
}

EditorFontManager::DefaultFontPath EditorFontManager::GetDefaultFontPath()
{
	FilePath defFontPath;
	FilePath defFontSpritePath;

	if (defaultFont)
	{
		Font::eFontType fontType = defaultFont->GetFontType();		
        switch (fontType)
        {
            case Font::TYPE_FT:
            {
                FTFont *ftFont = dynamic_cast<FTFont*>(defaultFont);
				FilePath ftFontPath = ftFont->GetFontPath();
				// Don't save standart default font
				if (ftFontPath.GetAbsolutePathname().find(DEFAULT_FONT_NAME) == String::npos)
				{
					// Set font path
					defFontPath = ftFontPath;
				}
                break;
            }
            case Font::TYPE_GRAPHICAL:
            {
                GraphicsFont *gFont = dynamic_cast<GraphicsFont*>(defaultFont);
                // Try to get font sprite
                Sprite *fontSprite = gFont->GetFontSprite();
				// Save font only if sprite is available
                if (fontSprite)
                {
					// Set font definition and sprite relative path
					defFontPath = gFont->GetFontDefinitionName();
					defFontSpritePath = fontSprite->GetRelativePathname();
                }
				break;
            }
        }	
	}
	
	return DefaultFontPath(defFontPath, defFontSpritePath);
}

void EditorFontManager::InitDefaultFontFromPath(const EditorFontManager::DefaultFontPath& defaultFontPath)
{
	FilePath fontPath = defaultFontPath.fontPath;
	FilePath fontSpritePath = defaultFontPath.fontSpritePath;
	Font* loadedFont = NULL;
	// Create font from loaded paths
	if (!fontPath.IsEmpty())
	{
		// Grpahics font
		if (!fontSpritePath.IsEmpty())
		{
            fontSpritePath.TruncateExtension();
			loadedFont = GraphicsFont::Create(fontPath, fontSpritePath);
		}
		else // True type font
		{
			loadedFont = FTFont::Create(fontPath);
		}
	}
	// Set default font only if font was really created
	if (loadedFont)
	{
		// Reset default font
		if (defaultFont)
		{
			defaultFont = NULL;
		}
		defaultFont = loadedFont;
	}	
}

QString EditorFontManager::GetDefaultFontName() const
{		
	if (defaultFont)
	{
		Font::eFontType fontType = defaultFont->GetFontType();
		switch (fontType)
		{
			case Font::TYPE_FT:
			{
				FTFont *ftFont = dynamic_cast<FTFont*>(defaultFont);
				return QString::fromStdString(ftFont->GetFontPath().GetAbsolutePathname());
			}
			case Font::TYPE_GRAPHICAL:
			{
				GraphicsFont *gFont = dynamic_cast<GraphicsFont*>(defaultFont);
				return QString::fromStdString(gFont->GetFontDefinitionName().GetAbsolutePathname());
			}
		}
	}
	return QString::fromStdString(DEFAULT_FONT_PATH);
}

const EditorFontManager::FONTSMAP& EditorFontManager::GetAllFonts() const
{
	return fonts;
}

QString EditorFontManager::GetFontName(Font* font) const
{
    if (font == NULL)
    {
        return QString();
    }

    EditorFontManager::FONTSMAP fonts = GetAllFonts();
    for (EditorFontManager::FONTSMAP::const_iterator iter = fonts.begin(); iter != fonts.end(); ++iter)
    {
        if (font->IsEqual(iter->second))
        {
            return iter->first.c_str();
        }
    }

    return QString();
}
