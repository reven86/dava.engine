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


#ifndef __EDITOR_FONT_SYSTEM__
#define __EDITOR_FONT_SYSTEM__

#include "Base/BaseTypes.h"
#include "Render/2d/Font.h"
#include <QObject>
#include <QStringList>

class EditorFontSystem: public QObject
{
Q_OBJECT
public:
    EditorFontSystem(QObject *parent = nullptr);
    ~EditorFontSystem();
    const char *GetDefaultFontLocale() const;
    DAVA::Font* GetFont(const DAVA::String &presetName, const DAVA::String &locale) const;
    void SetFont(const DAVA::String &presetName, const DAVA::String &locale, DAVA::Font *font);
    const QStringList &GetAvailableFontLocales() const;
    const QStringList &GetDefaultPresetNames() const;
    void LoadLocalizedFonts();
    void SaveLocalizedFonts();
    
    void ClearAllFonts();
    void CreateNewPreset(const DAVA::String &originalPresetName, const DAVA::String &newPresetName);                      
    
    void SetDefaultFontsPath(const DAVA::FilePath& path);
    DAVA::FilePath GetLocalizedFontsPath(const DAVA::String &locale) const;
    DAVA::FilePath GetDefaultFontsPath() const;
signals:
    void UpdateFontPreset();
public slots:
    void RegisterCurrentLocaleFonts();
private:
    void ClearFonts(DAVA::Map<DAVA::String, DAVA::Font*>& fonts);
    void RemoveFont(DAVA::Map<DAVA::String, DAVA::Font*> *fonts, const DAVA::String &fontName);
    void RefreshAvailableFontLocales();
    DAVA::FilePath defaultFontsPath;
       
    DAVA::Map<DAVA::String, DAVA::Map<DAVA::String, DAVA::Font*> > localizedFonts;

    QStringList availableFontLocales;
    QStringList defaultPresetNames;
    const char* defaultFontLocale;
    DAVA::String currentFontLocale; //wraps LocalizationSystem::currentLocale
};

inline const char* EditorFontSystem::GetDefaultFontLocale() const
{
    return defaultFontLocale;
}

inline const QStringList &EditorFontSystem::GetAvailableFontLocales() const
{
    return availableFontLocales;
}

inline const QStringList &EditorFontSystem::GetDefaultPresetNames() const
{
    return defaultPresetNames;
}

#endif //__EDITOR_FONT_SYSTEM__
