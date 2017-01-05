#pragma once

#include "EditorSystems/BaseEditorSystem.h"

namespace DAVA
{
class Vector2;
class UIControl;
}
class QScrollBar;

class EditorCanvasView final : public BaseEditorSystem
{
public:
    ScrollAreaController(EditorSystemsManager* parent);
    ~ScrollAreaController() override;

    void SetHorizontalScrollBar(QScrollBar* hScrollBar);
    void SetVerticalScrollBar(QScrollBar* vScrollBar);

private:
    void SetNestedControl(DAVA::UIControl* nestedControl);
    void SetMovableControl(DAVA::UIControl* movableControl);
    void AdjustScale(DAVA::float32 newScale, const DAVA::Vector2& mousePos);

    DAVA::Vector2 GetCanvasSize() const;
    DAVA::Vector2 GetViewSize() const;

    DAVA::Vector2 GetPosition() const;

    DAVA::float32 GetScale() const;
    DAVA::float32 GetMinScale() const;
    DAVA::float32 GetMaxScale() const;

    DAVA::Vector2 GetMinimumPos() const;
    DAVA::Vector2 GetMaximumPos() const;

    void SetViewSize(const DAVA::Vector2& size);
    void SetViewSize(DAVA::int32 width, DAVA::int32 height);

    void SetPosition(const DAVA::Vector2& position);
    void SetScale(DAVA::float32 scale);
    void UpdateCanvasContentSize();

    //private
    void UpdatePosition();
    DAVA::ScopedPtr<DAVA::UIControl> backgroundControl;
    DAVA::UIControl* nestedControl = nullptr;
    DAVA::UIControl* movableControl = nullptr;
    DAVA::Vector2 canvasSize = DAVA::Vector2(0.0f, 0.0f);
    DAVA::Vector2 viewSize = DAVA::Vector2(0.0f, 0.0f);
    DAVA::Vector2 position = DAVA::Vector2(0.0f, 0.0f);
    DAVA::float32 scale = 0.0f;
    const DAVA::float32 minScale = 0.25f;
    const DAVA::float32 maxScale = 8.0f;
    const DAVA::int32 Margin = 50;

    QScrollBar* hScrollBar = nullptr;
    QScrollBar* vScrollBar = nullptr;
    EditorSystemsManager::eDragState dragState = EditorSystemsManager::DragControls;
};
