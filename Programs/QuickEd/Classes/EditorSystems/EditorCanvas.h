#pragma once

#include "EditorSystems/BaseEditorSystem.h" 
#include "Functional/Signal.h"
#include "Base/ScopedPtr.h"

namespace DAVA
{
class Vector2;
class UIControl;
}

class EditorCanvas final : public BaseEditorSystem
{
public:
    EditorCanvas(DAVA::UIControl *nestedControl, DAVA::UIControl *movableControl, EditorSystemsManager* parent);
    ~EditorCanvas() override;

    bool CanProcessInput() const override;
    
    BaseEditorSystem::eInternalState RequireNewState(DAVA::UIEvent* currentInput) const override;
    void OnInput(DAVA::UIEvent* currentInput) override;
    
    DAVA::Vector2 GetCanvasSize() const;
    DAVA::Vector2 GetViewSize() const;
    
    DAVA::Vector2 GetPosition() const;
    
    DAVA::float32 GetScale() const;
    DAVA::float32 GetMinScale() const;
    DAVA::float32 GetMaxScale() const;
    
    DAVA::Vector2 GetMinimumPos() const;
    DAVA::Vector2 GetMaximumPos() const;
    void AdjustScale(DAVA::float32 newScale, const DAVA::Vector2& mousePos);


    void SetViewSize(const DAVA::Vector2& size);

    void SetPosition(const DAVA::Vector2& position);
    void SetScale(DAVA::float32 scale);

    DAVA::Signal<const DAVA::Vector2&> viewSizeChanged;
    DAVA::Signal<const DAVA::Vector2&> canvasSizeChanged;
    DAVA::Signal<const DAVA::Vector2&> positionChanged;
    DAVA::Signal<DAVA::float32> scaleChanged;

private:
    
    void UpdateCanvasContentSize();

    //private
    void UpdateDragScreenState();
    void UpdatePosition();
    DAVA::ScopedPtr<DAVA::UIControl> backgroundControl;
    DAVA::UIControl* nestedControl = nullptr;
    DAVA::UIControl* movableControl = nullptr;
    DAVA::Vector2 canvasSize = DAVA::Vector2(0.0f, 0.0f);
    DAVA::Vector2 viewSize = DAVA::Vector2(0.0f, 0.0f);
    DAVA::Vector2 position = DAVA::Vector2(0.0f, 0.0f);
    DAVA::float32 scale = 0.0f;

    DAVA::Vector2 lastMousePos;

    const DAVA::float32 minScale = 0.25f;
    const DAVA::float32 maxScale = 8.0f;
    const DAVA::float32 Margin = 50.0f;

    //helper members to store space button and left mouse buttons states
    bool isSpacePressed = false;
    bool isMouseLeftButtonPressed = false;
    bool isMouseMidButtonPressed = false;
};
