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

/**
 * \brief Class for creating screenshot of UIControl to Texture
 */
class UIScreenshoter
{
public:
    /**
    * \brief Default c-tor
    */
    UIScreenshoter();

    /**
    * \brief D-tor
    */
    ~UIScreenshoter();

    /**
    * \brief Call each frame to control internal resources and render flow
    */
    void OnFrame();

    /**
     * \brief Render control to texture and return it immediately (but not rendered) 
     * 
     * \param control pointer to source UIControl
     * \param format PixelFormat
     * \return target texture
     */
    Texture* MakeScreenshot(UIControl* control, const PixelFormat format);

    /**
     * \brief Render control to texture and call callback when it will rendered
     * 
     * \param control pointer to source UIControl
     * \param format PixelFormat
     * \param callback function which be called after render
     */
    void MakeScreenshot(UIControl* control, const PixelFormat format, Function<void(Texture*)> callback);

    /**
     * \brief Render control to target texture
     * 
     * \param control pointer to source UIControl
     * \param screenshot pointer to target Texture
     */
    void MakeScreenshot(UIControl* control, Texture* screenshot);

    /**
    * \brief Render control to target texture
    *
    * \param control pointer to source UIControl
    * \param screenshot pointer to target Texture
    * \param callback function which be called after render
    */
    void MakeScreenshot(UIControl* control, Texture* screenshot, Function<void(Texture*)> callback);

private:
    struct Control3dInfo
    {
        UI3DView* control = nullptr;
        rhi::RenderPassConfig scenePassConfig;
    };

    struct ScreenshotWaiter
    {
        Texture * texture = nullptr;
        rhi::HSyncObject syncObj;
        Function<void(Texture*)> callback;
    };

    void MakeScreenshotInternal(UIControl* control, Texture* screenshot, Function<void(Texture*)> callback);
    void FindAll3dViews(UIControl * control, List<UIScreenshoter::Control3dInfo> & foundViews);
    void RenderToTexture(const ScreenshotWaiter& waiter);
    
    List<ScreenshotWaiter> waiters;
};

};

#endif //__DAVEAENGINE_UI_SCREENSHOTER__