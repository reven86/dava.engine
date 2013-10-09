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



#ifndef __TEXTURE_FORMAT_DIALOG_H__
#define __TEXTURE_FORMAT_DIALOG_H__

#include "DAVAEngine.h"
#include "ExtendedDialog.h"

using namespace DAVA;

class TextureFormatDialogDelegate
{
public:
    virtual void OnFormatSelected(PixelFormat newFormat, bool generateMimpaps) = 0;
};

class UICheckBox;
class TextureFormatDialog: public ExtendedDialog
{
public:
    TextureFormatDialog(TextureFormatDialogDelegate *newDelegate);
    virtual ~TextureFormatDialog();

    void Show();

protected:

    virtual const Rect GetDialogRect() const;

    UIButton *closeButtonTop;
    void OnCancel(BaseObject * owner, void * userData, void * callerData);

    UIButton *convertButton;
    void OnConvert(BaseObject * owner, void * userData, void * callerData);
    
    TextureFormatDialogDelegate *delegate;

    //PVR
    enum ePVRButtons
    {
        PVR_NONE = -1,
        PVRTC4 = 0,
        PVRTC2,
        
        PVR_COUNT
    };
    
    ePVRButtons currentPVRButton;
    UIButton *pvrButtons[PVR_COUNT];
    void OnPVRButton(BaseObject * owner, void * userData, void * callerData);
    
    //MIPMAPS
    UICheckBox *mipmapEnabled;
};



#endif // __TEXTURE_FORMAT_DIALOG_H__