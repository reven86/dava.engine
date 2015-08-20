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


#ifndef __UIEditor_EditScreen_h__
#define __UIEditor_EditScreen_h__

#include "DAVAEngine.h"

class ControlSelectionListener;

class CheckeredCanvas: public DAVA::UIControl
{
public:
    CheckeredCanvas();
private:
    virtual ~CheckeredCanvas();
    
public:
    virtual void Draw(const DAVA::UIGeometricData &geometricData) override;
    virtual void DrawAfterChilds(const DAVA::UIGeometricData &geometricData) override;
    virtual bool SystemInput(DAVA::UIEvent *currentInput) override;

public:
    void SelectControl(UIControl *control);
    void RemoveSelection(UIControl *control);
    void ClearSelections();
    UIControl *GetControlByPos(UIControl *control, const DAVA::Vector2 &pos);
    
    void AddControlSelectionListener(ControlSelectionListener *listener);
    void RemoveControlSelectionListener(ControlSelectionListener *listener);
    void SetEmulationMode(bool emulationMode);

private:
    DAVA::Set<UIControl*> selectionControls;
    DAVA::List<ControlSelectionListener*> selectionListeners;
    bool emulationMode = false;
};

class PackageCanvas: public DAVA::UIControl
{
public:
    PackageCanvas();
private:
    virtual ~PackageCanvas();
public:
    void LayoutCanvas();
private:
};

#endif // __UIEditor_EditScreen_h__
