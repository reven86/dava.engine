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



#ifndef __UIEditor__FontManager__
#define __UIEditor__FontManager__

#include <iostream>
#include <QObject>
#include <DAVAEngine.h>

class EditorFontManager: public QObject, public DAVA::Singleton<EditorFontManager>
{
    Q_OBJECT
public:
	//typedef std::map<String, Font*> FONTSMAP;
	EditorFontManager();
	~EditorFontManager();
	
	void Reset();

    void LoadLocalizedFonts();
    void SaveLocalizedFonts();
    
    void ClearLocalizedFonts();
    
    void OnProjectLoaded();

    DAVA::Font* GetDefaultFont() const;
    void SetDefaultFont(DAVA::Font* font);
	void ResetDefaultFont();
    
	//Font* GetFont(const String& name) const;
    //QString GetFontName(Font* font) const;
    
    const DAVA::Vector<DAVA::String> &GetLocales() { return locales; }
    
    DAVA::String GetFontDisplayName(DAVA::Font* font);
    
    const DAVA::Map<DAVA::String, DAVA::Font*> &GetLocalizedFonts(const DAVA::String& locale) const;
    DAVA::Font* GetLocalizedFont(const DAVA::String& fontName, const DAVA::String& locale) const;
    DAVA::String GetLocalizedFontName(DAVA::Font* font) const;
    DAVA::Font* GetLocalizedFont(DAVA::Font* font) const;
    
    DAVA::String SetLocalizedFont(const DAVA::String& fontOriginalName, DAVA::Font* font, const DAVA::String& fontName, bool replaceExisting, const DAVA::String& locale);

	//const FONTSMAP& GetAllFonts() const;
	
	struct DefaultFontPath
	{
        DAVA::FilePath fontPath;
        DAVA::FilePath fontSpritePath;
		
        DefaultFontPath(const DAVA::FilePath & fontPath, const DAVA::FilePath & fontSpritePath)
		{
			this->fontPath = fontPath;
			this->fontSpritePath = fontSpritePath;
		}
	};
	
	DefaultFontPath GetDefaultFontPath();
	void InitDefaultFontFromPath(const DefaultFontPath& defaultFontPath);
	QString GetDefaultFontName() const;
    
    void SetProjectDataPath(const DAVA::FilePath& path);
    void SetDefaultFontsPath(const DAVA::FilePath& path);
    DAVA::FilePath GetLocalizedFontsPath(const DAVA::String &locale);
    const DAVA::FilePath& GetDefaultFontsPath();
    DAVA::String GetDefaultFontsFrameworkPath();
    
signals:
    void FontLoaded();
	
private:
	void Init();
    
    DAVA::Font* CreateDefaultFont(const DAVA::String& fontPath, const DAVA::String& fontName);
    void RegisterDefaultFont(DAVA::Font* font);
    
    void ClearFonts(DAVA::Map<DAVA::String, DAVA::Font*>& fonts);
    
    void ResetLocalizedFontsPath();
	
private:
    DAVA::Font* defaultFont;
    DAVA::Font* baseFont;
    
    DAVA::FilePath projectDataPath;
    DAVA::FilePath defaultFontsPath;
    
    DAVA::Vector<DAVA::String> locales;
    
    DAVA::Map<DAVA::String, DAVA::Font*> defaultFonts;
    DAVA::Map<DAVA::String, DAVA::Map<DAVA::String, DAVA::Font*> > localizedFonts;
    
    DAVA::Map<DAVA::Font*, DAVA::String> defaultRegisteredFonts;
};

#endif /* defined(__UIEditor__FontManager__) */
