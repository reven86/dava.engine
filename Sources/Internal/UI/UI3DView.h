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


#ifndef __DAVAENGINE_UI_3D_VIEW__
#define __DAVAENGINE_UI_3D_VIEW__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"

namespace DAVA
{
/**
    \ingroup controlsystem
    \brief This control allow to put 3D View into any place of 2D hierarchy
 */

class Scene;
class UI3DView : public UIControl
{
public:
    UI3DView(const Rect& rect = Rect());

protected:
    virtual ~UI3DView();
public:
    void SetScene(Scene * scene);
    Scene * GetScene() const;

    inline const Rect& GetLastViewportRect()
    {
        return viewportRc;
    }

    void AddControl(UIControl* control) override;
    void Update(float32 timeElapsed) override;
    void Draw(const UIGeometricData& geometricData) override;

    void WillBecomeVisible() override;
    void WillBecomeInvisible() override;

    void SetSize(const Vector2& newSize) override;
    UI3DView* Clone() override;

    void Input(UIEvent* currentInput) override;

protected:
    Scene * scene;
    Rect viewportRc;
    bool registeredInUIControlSystem;

public:
    INTROSPECTION_EXTEND(UI3DView, UIControl,
                         nullptr);
};
};

#endif
