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

#ifndef __DAVEAENGINE_UI_SCREENSHOTER__
#define __DAVEAENGINE_UI_SCREENSHOTER__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "UI/UIControl.h"

namespace DAVA
{

class UIControl;
class UI3DView;
class Texture;

class UIScreenshoter
{
public:
    UIScreenshoter();
    ~UIScreenshoter();

    Texture* MakeScreenshot(UIControl* control, const PixelFormat format, bool cloneControl = false);
    void MakeScreenshotWithCallback(UIControl* control, const PixelFormat format, Function<void(Texture*)> callback, bool cloneControl = false);
    void MakeScreenshotWithPreparedTexture(UIControl* control, Texture* texture, bool cloneControl = false);

    void OnFrame();

private:
    struct Control3dInfo
    {
        UI3DView* control;
        int32 priority;
        rhi::Handle texture;
    };

    struct ScreenshotWaiter
    {
        UIControl * control;
        Texture * texture;
        int32 cooldown;
        Function<void(Texture*)> callback;
    };

    List<Control3dInfo> FindAll3dViews(UIControl * control);
    void RenderToTexture(UIControl* control, Texture* screenshot);
    
    List<ScreenshotWaiter> waiters;
};

};

#endif //__DAVEAENGINE_UI_SCREENSHOTER__