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



#ifndef __LANDSCAPE_EDITOR_PROPERTY_CONTROL_H__
#define __LANDSCAPE_EDITOR_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "LandscapePropertyControl.h"

class LandscapeEditorSettings: public BaseObject
{
protected:
    ~LandscapeEditorSettings(){}
public:
    
    LandscapeEditorSettings(); 
    void ResetAll();
    
    bool redMask;
    bool greenMask;
    bool blueMask;
    bool alphaMask;
    
    int32 maskSize;
};

class LandscapeEditorPropertyControlDelegate
{
public: 
    virtual void LandscapeEditorSettingsChanged(LandscapeEditorSettings *settings) = 0;
    virtual void TextureWillChanged(const String &forKey) = 0;
    virtual void TextureDidChanged(const String &forKey) = 0;
};

class LandscapeEditorPropertyControl: public LandscapePropertyControl
{
public:
    
    enum eEditorMode
    {
        MASK_EDITOR_MODE = 0,
        HEIGHT_EDITOR_MODE,
		COLORIZE_EDITOR_MODE
    };
    
	LandscapeEditorPropertyControl(const Rect & rect, bool createNodeProperties, eEditorMode mode);
	virtual ~LandscapeEditorPropertyControl();

    virtual void Input(UIEvent *currentInput);

    
	virtual void ReadFrom(Entity * sceneNode);
    virtual void OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const FilePath &newValue);
    virtual void OnTexturePreviewPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);

    void SetDelegate(LandscapeEditorPropertyControlDelegate *newDelegate);
    
    LandscapeEditorSettings * Settings();
    
protected:

    void SetValuesFromSettings();
    
    LandscapeEditorSettings *settings;
    LandscapeEditorPropertyControlDelegate *delegate;
    
    eEditorMode editorMode;
};

#endif //__LANDSCAPE_PROPERTY_CONTROL_H__
