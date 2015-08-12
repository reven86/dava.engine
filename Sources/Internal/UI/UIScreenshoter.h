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
     * \brief Set priorities for render passes (bigger - higher)
     * 
     * \param clearPriority priority for clear pass
     * \param pass3dPriority priority for 3d pass
     * \param pass2dPriority priority for 2d/ui pass
     */
    void SetRenderPriorities(int32 clearPriority, int32 pass3dPriority, int32 pass2dPriority);

    /**
     * \brief Set frames count for clearing internal resources and call callbacks
     * 
     * \param cooldown frames count
     */
    void SetCooldownFrames(int32 cooldown);

    /**
     * \brief Render control to texture and return it immediately (but not rendered) 
     * 
     * \param control pointer to source UIControl
     * \param format PixelFormat
     * \param cloneControl if true when shooter create clone of control for save it state
     * \return target texture
     */
    Texture* MakeScreenshot(UIControl* control, const PixelFormat format, bool cloneControl = false);

    /**
     * \brief Render control to texture and call callback when it will rendered
     * 
     * \param control pointer to source UIControl
     * \param format PixelFormat
     * \param callback function which be called after render
     * \param cloneControl if true when shooter create clone of control for save it state
     */
    void MakeScreenshotWithCallback(UIControl* control, const PixelFormat format, Function<void(Texture*)> callback, bool cloneControl = false);

    /**
     * \brief Render control to target texture
     * 
     * \param control pointer to source UIControl
     * \param screenshot pointer to target Texture
     * \param cloneControl if true when shooter create clone of control for save it state
     */
    void MakeScreenshotWithPreparedTexture(UIControl* control, Texture* screenshot, bool cloneControl = false);

    /**
     * \brief Call each frame to control internal resources and render flow
     */
    void OnFrame();

private:
    struct Control3dInfo
    {
        UI3DView* control = nullptr;
        int32 priority = 0;
        rhi::Handle texture = rhi::InvalidHandle;
        rhi::Handle depht = rhi::InvalidHandle;
    };

    struct ScreenshotWaiter
    {
        UIControl * control = nullptr;
        Texture * texture = nullptr;
        rhi::HTexture depht;
        int32 cooldown = 0;
        Function<void(Texture*)> callback;

        void Free();
    };

    void MakeScreenshotInternal(UIControl* control, Texture* screenshot, Function<void(Texture*)> callback, bool cloneControl);
    void FindAll3dViews(UIControl * control, List<UIScreenshoter::Control3dInfo> & foundViews);
    void RenderToTexture(const ScreenshotWaiter& waiter);
    
    List<ScreenshotWaiter> waiters;
    int32 clearPriority;
    int32 pass3dPriority;
    int32 pass2dPriority;
    int32 cooldownFrames;
};

inline void UIScreenshoter::SetRenderPriorities(int32 _clearPriority, int32 _pass3dPriority, int32 _pass2dPriority)
{
    clearPriority = _clearPriority;
    pass2dPriority = _pass2dPriority;
    pass3dPriority = _pass3dPriority;
}

inline void UIScreenshoter::SetCooldownFrames(int32 _cooldown)
{
    cooldownFrames = _cooldown;
}

};

#endif //__DAVEAENGINE_UI_SCREENSHOTER__