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


#ifndef UIEditor_EditorSettings_h
#define UIEditor_EditorSettings_h

#include "DAVAEngine.h"

class EditorSettings: public DAVA::Singleton<EditorSettings>
{

public: 
    enum eDefaultSettings
    {
        RECENT_FILES_COUNT = 5,
    };
	
public:
	EditorSettings();
    virtual ~EditorSettings();

    DAVA::KeyedArchive *GetSettings();
    void Save();
	
    void SetProjectPath(const DAVA::String &projectPath);
    DAVA::String GetProjectPath();
	
    DAVA::int32 GetLastOpenedCount();
    DAVA::String GetLastOpenedFile(DAVA::int32 index);
    void AddLastOpenedFile(const DAVA::String & pathToFile);

    void SetUIEditorVersion(const DAVA::String& editorVersion);
    DAVA::String GetUIEditorVersion();

    // Whether "Pixelized" or "Smooth" images representation is used.
    void SetPixelized(bool value);
    bool IsPixelized() const;

    DAVA::Color GetGrigColor() const;
    void SetGrigColor(const DAVA::Color& color);

    // Background Frame colors.
    DAVA::Color GetCurrentBackgroundFrameColor() const;
    void SetCurrentBackgroundFrameColor(const DAVA::Color& color);
    
    DAVA::Color GetCustomBackgroundFrameColor() const;
    void SetCustomBackgroundFrameColor(const DAVA::Color& color);

    bool IsUsingAssetCache() const;
    DAVA::String GetAssetCacheIp() const;
    DAVA::String GetAssetCachePort() const;
    DAVA::String GetAssetCacheTimeoutSec() const;

protected:
    DAVA::Color GetColor(const DAVA::String& colorName, const DAVA::Color& defaultColor) const;
    void SetColor(const DAVA::String& colorName, const DAVA::Color& color);

private:
    DAVA::KeyedArchive *settings;
};

#endif //UIEditor_EditorSettings_h
