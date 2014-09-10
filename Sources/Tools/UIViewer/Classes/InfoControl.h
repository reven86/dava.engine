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


//
//  InfoControl.h
//  UIViewerMacOS
//
//  Created by Dmitry Shpakov on 6/29/12.
//  Copyright (c) 2012 DAVA Consulting. All rights reserved.
//

#ifndef __INFO_CONTROL_H__
#define __INFO_CONTROL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class InfoControl : public UIControl
{
protected:
    virtual ~InfoControl();
public:
    InfoControl(UIControl* watched);
    
    enum eInfoTypes
    {
        IT_TYPE = 0,
        IT_NAME,
        IT_RECT,
        IT_PIVOT,
        
        INFO_TYPES_COUNT
    };
    
    WideString GetInfoValueText(int32 infoType);
    
    int32 GetSpecificInfoCount();
    
    const WideString &GetSpecificInfoKeyText(int32 index);
    const WideString &GetSpecificInfoValueText(int32 index);
    
    virtual void Draw(const UIGeometricData &geometricData);
    
protected:
    virtual void ParseSpecificInfo();
    
    UIControl* watchedControl;
    
    WideString emptyString;
    
    Vector<WideString> specificInfoKeys;
    Vector<WideString> specificInfoValues;
};

#endif //__INFO_CONTROL_H__
