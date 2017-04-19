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
    EditorCanvas(EditorSystemsManager* parent, DAVA::TArc::ContextAccessor* accessor);
    ~EditorCanvas() override;

    bool CanProcessInput(DAVA::UIEvent* currentInput) const override;
    EditorSystemsManager::eDragState RequireNewState(DAVA::UIEvent* currentInput) override;
    void ProcessInput(DAVA::UIEvent* currentInput) override;

    DAVA::Vector2 GetSize() const;
    DAVA::Vector2 GetViewSize() const;
    void OnViewSizeChanged(DAVA::uint32 width, DAVA::uint32 height);

    DAVA::float32 GetScale() const;
    DAVA::float32 GetMinScale() const;
    DAVA::float32 GetMaxScale() const;
    void AdjustScale(DAVA::float32 newScale, const DAVA::Vector2& mousePos);
    void SetScale(DAVA::float32 scale);

    DAVA::Vector2 GetPosition() const;
    DAVA::Vector2 GetMinimumPos() const;
    DAVA::Vector2 GetMaximumPos() const;
    void SetPosition(const DAVA::Vector2& position);

    const DAVA::Vector<DAVA::float32>& GetPredefinedScales() const;

    DAVA::float32 GetNextScale(DAVA::int32 ticksCount) const;
    DAVA::float32 GetPreviousScale(DAVA::int32 ticksCount) const;

    DAVA::Signal<const DAVA::Vector2&> sizeChanged;
    DAVA::Signal<const DAVA::Vector2&> positionChanged;
    DAVA::Signal<const DAVA::Vector2&> nestedControlPositionChanged; //control position, excluding margins
    DAVA::Signal<DAVA::float32> scaleChanged;

private:
    void OnContentSizeChanged(const DAVA::Vector2& size);

    void UpdateContentSize();
    void UpdateDragScreenState();
    void UpdatePosition();

    DAVA::float32 GetScaleFromWheelEvent(DAVA::int32 ticksCount) const;

    DAVA::UIControl* movableControl = nullptr;
    DAVA::Vector2 size = DAVA::Vector2(0.0f, 0.0f);
    DAVA::Vector2 contentSize = DAVA::Vector2(0.0f, 0.0f);
    DAVA::Vector2 viewSize = DAVA::Vector2(0.0f, 0.0f);
    DAVA::Vector2 position = DAVA::Vector2(0.0f, 0.0f);
    DAVA::float32 scale = 0.0f;

    const DAVA::float32 minScale = 0.25f;
    const DAVA::float32 maxScale = 8.0f;
    const DAVA::float32 margin = 50.0f;
    bool isMouseMidButtonPressed = false;
    bool isSpacePressed = false;

    DAVA::Vector<DAVA::float32> predefinedScales;
};
