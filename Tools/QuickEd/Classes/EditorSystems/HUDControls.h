#pragma once

#include "UI/UIControl.h"
#include "EditorSystemsManager.h"

class VisibleValueProperty;

class ControlContainer : public DAVA::UIControl
{
public:
    explicit ControlContainer(const HUDAreaInfo::eArea area);
    HUDAreaInfo::eArea GetArea() const;
    virtual void InitFromGD(const DAVA::UIGeometricData& gd_) = 0;
    void SetSystemVisible(bool visible);
    bool GetSystemVisible() const;

protected:
    ~ControlContainer() = default;
    const HUDAreaInfo::eArea area = HUDAreaInfo::NO_AREA;
    bool systemVisible = true;
};

class HUDContainer : public ControlContainer
{
public:
    explicit HUDContainer(ControlNode* node);
    void AddChild(ControlContainer* container);
    void InitFromGD(const DAVA::UIGeometricData& geometricData) override;
    void SystemDraw(const DAVA::UIGeometricData& geometricData) override;

private:
    ~HUDContainer() = default;
    ControlNode* node = nullptr;
    VisibleValueProperty* visibleProperty = nullptr;
    DAVA::UIControl* control = nullptr;
    DAVA::Vector<DAVA::RefPtr<ControlContainer>> childs;
};

class FrameControl : public ControlContainer
{
public:
    enum eBorder
    {
        TOP,
        BOTTOM,
        LEFT,
        RIGHT,
        COUNT
    };

    enum eType
    {
        CHECKERED,
        UNIFORM
    };
    explicit FrameControl(eType type = CHECKERED);

protected:
    ~FrameControl() = default;
    void InitFromGD(const DAVA::UIGeometricData& geometricData) override;
    eType type = CHECKERED;
};

class FrameRectControl : public ControlContainer
{
public:
    explicit FrameRectControl(const HUDAreaInfo::eArea area_);

private:
    ~FrameRectControl() = default;
    void InitFromGD(const DAVA::UIGeometricData& geometricData) override;
    DAVA::Vector2 GetPos(const DAVA::UIGeometricData& geometricData) const;
};

class PivotPointControl : public ControlContainer
{
public:
    explicit PivotPointControl();

private:
    ~PivotPointControl() = default;
    void InitFromGD(const DAVA::UIGeometricData& geometricData) override;
};

class RotateControl : public ControlContainer
{
public:
    explicit RotateControl();

private:
    ~RotateControl() = default;
    void InitFromGD(const DAVA::UIGeometricData& geometricData) override;
};

void SetupHUDMagnetLineControl(DAVA::UIControl* control);
void SetupHUDMagnetRectControl(DAVA::UIControl* control);

DAVA::UIControl* CreateHUDRect(ControlNode* node);
