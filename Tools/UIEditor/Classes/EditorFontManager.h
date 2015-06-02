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
using namespace DAVA;

class EditorFontManager: public QObject, public Singleton<EditorFontManager>
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

	Font* GetDefaultFont() const;
	void SetDefaultFont(Font* font);
	void ResetDefaultFont();
    
	//Font* GetFont(const String& name) const;
    //QString GetFontName(Font* font) const;
    
    const Vector<String> &GetLocales() { return locales; }
    
    String GetFontDisplayName(Font* font);
    
    const Map<String, Font*> &GetLocalizedFonts(const String& locale) const;
    Font* GetLocalizedFont(const String& fontName, const String& locale) const;
    String GetLocalizedFontName(Font* font) const;
    Font* GetLocalizedFont(Font* font) const;
    
    String SetLocalizedFont(const String& fontOriginalName, Font* font, const String& fontName, bool replaceExisting, const String& locale);

	//const FONTSMAP& GetAllFonts() const;
	
	struct DefaultFontPath
	{
		FilePath fontPath;
		FilePath fontSpritePath;
		
		DefaultFontPath(const FilePath & fontPath, const FilePath & fontSpritePath)
		{
			this->fontPath = fontPath;
			this->fontSpritePath = fontSpritePath;
		}
	};
	
	DefaultFontPath GetDefaultFontPath();
	void InitDefaultFontFromPath(const DefaultFontPath& defaultFontPath);
	QString GetDefaultFontName() const;
    
    void SetProjectDataPath(const FilePath& path);
    void SetDefaultFontsPath(const FilePath& path);
    FilePath GetLocalizedFontsPath(const String &locale);
    const FilePath& GetDefaultFontsPath();
    String GetDefaultFontsFrameworkPath();
    
signals:
    void FontLoaded();
	
private:
	void Init();
    
	Font* CreateDefaultFont(const String& fontPath, const String& fontName);
    void RegisterDefaultFont(Font* font);
    
    void ClearFonts(Map<String, Font*>& fonts);
    
    void ResetLocalizedFontsPath();
	
private:
	Font* defaultFont;
	Font* baseFont;
    
    FilePath projectDataPath;
    FilePath defaultFontsPath;
    
    Vector<String> locales;
    
    Map<String, Font*> defaultFonts;
    Map<String, Map<String, Font*> > localizedFonts;
    
    Map<Font*, String> defaultRegisteredFonts;
};

#endif /* defined(__UIEditor__FontManager__) */
