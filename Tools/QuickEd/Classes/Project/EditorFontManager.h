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

#include "Base/BaseTypes.h"
#include "Render/2d/Font.h"
#include <QObject>

class EditorFontManager: public QObject
{
Q_OBJECT
public:
	//typedef std::map<String, Font*> FONTSMAP;
	EditorFontManager(QObject *parent = nullptr);
	~EditorFontManager() = default;
	
    void LoadLocalizedFonts();
    void SaveLocalizedFonts();
    
    void ClearLocalizedFonts();
           
    const DAVA::Map<DAVA::String, DAVA::Font*> &GetLocalizedFonts(const DAVA::String& locale) const;
    DAVA::Font* GetLocalizedFont(const DAVA::String& fontName, const DAVA::String& locale) const;
    
    DAVA::String UseNewPreset(const DAVA::String& fontOriginalName, DAVA::Font* font, const DAVA::String& fontName, const DAVA::String& locale);
    DAVA::String CreateNewPreset(const DAVA::String &presetName, DAVA::Font *font, const DAVA::String &locale);                      
    
    void SetDefaultFontsPath(const DAVA::FilePath& path);
    DAVA::FilePath GetLocalizedFontsPath(const DAVA::String &locale);
    DAVA::FilePath GetDefaultFontsPath();
signals:
    void UpdateFontPreset(const QString &oldPresetName, const QString &newPresetName);

private:
    void ClearFonts(DAVA::Map<DAVA::String, DAVA::Font*>& fonts);
    DAVA::Vector<DAVA::String> GetAvailableFonts() const;

private:
   
    DAVA::FilePath defaultFontsPath;
       
    DAVA::Map<DAVA::String, DAVA::Font*> defaultFonts;
    DAVA::Map<DAVA::String, DAVA::Map<DAVA::String, DAVA::Font*> > localizedFonts;
};

#endif /* defined(__UIEditor__FontManager__) */
