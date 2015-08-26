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


#ifndef __BASESCREEN_H__
#define __BASESCREEN_H__

#include "DAVAEngine.h"
#include "Infrastructure/GameCore.h"

class BaseScreen : public DAVA::UIScreen
{
protected:
    virtual ~BaseScreen(){}
public:

    BaseScreen();
    BaseScreen(const DAVA::String & screenName, DAVA::int32 skipBeforeTests = 10);
    
    inline DAVA::int32 GetScreenId();
    
    void SystemScreenSizeDidChanged(const DAVA::Rect &newFullScreenSize) override;

protected:
    UIButton *exitButton;

    void LoadResources() override;
    void UnloadResources() override;
    bool SystemInput(DAVA::UIEvent *currentInput) override;

    virtual void OnExitButton(DAVA::BaseObject *obj, void *data, void *callerData);
    
private:
    static DAVA::int32 globalScreenId; // 1, on create of screen increment  
    DAVA::int32 currentScreenId;
    DAVA::UIButton *exitButton;
};

DAVA::int32 BaseScreen::GetScreenId()
{
    return currentScreenId;
}


#endif // __BASESCREEN_H__
