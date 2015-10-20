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


#ifndef _QUIECKED_EDITOR_SYSTEMS_HUD_CONTROLS_H_
#define _QUIECKED_EDITOR_SYSTEMS_HUD_CONTROLS_H_

#include "UI/UIControl.h"
#include "EditorSystemsManager.h"

class ControlContainer : public DAVA::UIControl
{
public:
    explicit ControlContainer(const HUDAreaInfo::eArea area);
    HUDAreaInfo::eArea GetArea() const;
    virtual void InitFromGD(const DAVA::UIGeometricData& gd_) = 0;

protected:
    const HUDAreaInfo::eArea area = HUDAreaInfo::NO_AREA;
};

class HUDContainer : public ControlContainer
{
public:
    explicit HUDContainer(DAVA::UIControl* container);
    void AddChild(ControlContainer* container);
    void InitFromGD(const DAVA::UIGeometricData& geometricData) override;
    void SystemDraw(const DAVA::UIGeometricData& geometricData) override;
    void SetVisibleInSystems(bool arg);
    void SetValid(bool arg);

private:
    DAVA::UIControl* control = nullptr;
    DAVA::Vector<DAVA::RefPtr<ControlContainer>> childs;
    bool valid = false;
    bool visibleInSystems = true;
};

template <typename T>
T* CreateContainerWithBorders();

class FrameControl : public ControlContainer
{
    template <typename T>
    friend T* CreateContainerWithBorders();

public:
    enum
    {
        BORDER_TOP,
        BORDER_BOTTOM,
        BORDER_LEFT,
        BORDER_RIGHT,
        BORDERS_COUNT
    };
    void Init();

protected:
    explicit FrameControl();
    void InitFromGD(const DAVA::UIGeometricData& geometricData) override;
    DAVA::Rect CreateFrameBorderRect(DAVA::uint32 border, const DAVA::Rect& frameRect) const;
};

class FrameRectControl : public ControlContainer
{
public:
    explicit FrameRectControl(const HUDAreaInfo::eArea area_);

private:
    void InitFromGD(const DAVA::UIGeometricData& geometricData) override;
    DAVA::Vector2 GetPos(const DAVA::UIGeometricData& geometricData) const;
};

class PivotPointControl : public ControlContainer
{
public:
    explicit PivotPointControl();

private:
    void InitFromGD(const DAVA::UIGeometricData& geometricData) override;
};

class RotateControl : public ControlContainer
{
public:
    explicit RotateControl();

private:
    void InitFromGD(const DAVA::UIGeometricData& geometricData) override;
};

class SelectionRect : public FrameControl
{
    template <typename T>
    friend T* CreateContainerWithBorders();
    void Draw(const DAVA::UIGeometricData& geometricData) override;
};

template <typename T>
T* CreateContainerWithBorders()
{
    static_assert(std::is_base_of<FrameControl, T>::value, "works only for classes, based on FrameControl");
    T* t = new T;
    t->Init();
    return t;
}

#endif //_QUIECKED_EDITOR_SYSTEMS_HUD_CONTROLS_H_
