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


#ifndef __QUICKED_HUD_SYSTEM_H__
#define __QUICKED_HUD_SYSTEM_H__

#include "Interfaces.h"
#include "Base/ScopedPtr.h"
#include "Math/Vector.h"
#include "Math/Rect.h"
#include "UI/UIControl.h"
#include "Systems/BaseSystem.h"
#include "Functional/Signal.h"

class Document;

class ControlContainer : public DAVA::UIControl
{
public:
    explicit ControlContainer(const ControlAreaInterface::eArea area);

    ControlAreaInterface::eArea GetArea() const;
    virtual void InitFromGD(const DAVA::UIGeometricData &gd_) = 0;

protected:
    const ControlAreaInterface::eArea area = ControlAreaInterface::NO_AREA;
};

class HUDSystem final : public BaseSystem, public InputInterface
{
public:
    HUDSystem(Document *document);
    ~HUDSystem() = default;
    void AttachToRoot(DAVA::UIControl *root);
    void Detach() override;

    void OnSelectionWasChanged(const SelectedControls &selected, const SelectedControls &deselected);
    bool OnInput(DAVA::UIEvent *currentInput) override;

    void AddListener(ControlAreaInterface *listener);
    void RemoveListener(ControlAreaInterface *listener);

    DAVA::Signal<const DAVA::Rect &/*selectionRectControl*/> SelectionRectChanged;

private:
    void ProcessCursor(const DAVA::Vector2& pos);
    void GetControlArea(ControlNode *&node, ControlAreaInterface::eArea &area, const DAVA::Vector2 &pos);
    void SetNewArea(ControlNode* node, const ControlAreaInterface::eArea area);

    ControlAreaInterface::eArea activeArea = ControlAreaInterface::NO_AREA;
    ControlNode *activeControl = nullptr;

    DAVA::ScopedPtr<DAVA::UIControl> hudControl;

    DAVA::Vector2 pressedPoint; //corner of selection rect
    bool canDrawRect = false; //selection rect state

    DAVA::List<ControlAreaInterface*> listeners;

    struct HUD
    {
        HUD(const Document *doc, ControlNode *node, DAVA::UIControl *hudControl);
        ~HUD();
        ControlNode *node = nullptr;
        DAVA::UIControl *control = nullptr;
        DAVA::ScopedPtr<ControlContainer> container;
        DAVA::Vector < DAVA::ScopedPtr<ControlContainer> > hudControls;
    };
    DAVA::Map<ControlNode*, HUD> hudMap;
    DAVA::ScopedPtr<DAVA::UIControl> selectionRectControl;
};

#endif // __QUICKED_HUD_SYSTEM_H__
