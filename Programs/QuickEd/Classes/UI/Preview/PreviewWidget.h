#pragma once

#include "EditorSystems/EditorSystemsManager.h"

#include <Engine/Qt/IClientDelegate.h>
#include <Engine/Qt/RenderWidget.h>

#include <QFrame>
#include <QCursor>
#include <QPointer>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
class DataContext;
}
}
class EditorSystemsManager;
class WidgetsData;

class ControlNode;
class PackageBaseNode;
class CursorInterpreter;
class AbstractProperty;

class FindInDocumentWidget;
class RulerWidget;
class RulerController;
class RulerController;
class EditorCanvas;
class GuidesController;

class QGridLayout;
class QComboBox;
class QScrollBar;
class RulerController;
class QWheelEvent;
class QNativeGestureEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QMenu;

class PreviewWidget : public QFrame, private DAVA::IClientDelegate
{
    Q_OBJECT
public:
    explicit PreviewWidget(DAVA::TArc::ContextAccessor* accessor, DAVA::RenderWidget* renderWidget, EditorSystemsManager* systemsManager);
    ~PreviewWidget();

    FindInDocumentWidget* GetFindInDocumentWidget();

    DAVA::Signal<DAVA::uint64> requestCloseTab;
    DAVA::Signal<ControlNode*> requestChangeTextInNode;

signals:
    void DeleteRequested();
    void ImportRequested();
    void CutRequested();
    void CopyRequested();
    void PasteRequested();
    void DuplicateRequested();

    void OpenPackageFile(QString path);
    void DropRequested(const QMimeData* data, Qt::DropAction action, PackageBaseNode* targetNode, DAVA::uint32 destIndex, const DAVA::Vector2* pos);

public slots:
    void OnRootControlPositionChanged(const DAVA::Vector2& pos);
    void OnNestedControlPositionChanged(const DAVA::Vector2& pos);
    void OnEmulationModeChanged(bool emulationMode);
    void OnIncrementScale();
    void OnDecrementScale();
    void SetActualScale();

private slots:
    void OnScaleChanged(DAVA::float32 scale);
    void OnScaleByComboIndex(int value);
    void OnScaleByComboText();

    void OnVScrollbarActionTriggered(int action);
    void OnHScrollbarActionTriggered(int action);

    //function argument used for signals compatibility
    void UpdateScrollArea(const DAVA::Vector2& size = DAVA::Vector2(0.0f, 0.0f));
    void OnPositionChanged(const DAVA::Vector2& position);
    void OnResized(DAVA::uint32 width, DAVA::uint32 height);
    void OnRulersGeometryChanged();

private:
    void InitUI();
    void ShowMenu(const QMouseEvent* mouseEvent);
    bool AddSelectionMenuSection(QMenu* parentMenu, const QPoint& pos);
    bool CanChangeTextInControl(const ControlNode* node) const;

    void InitFromSystemsManager(EditorSystemsManager* systemsManager);

    void InjectRenderWidget(DAVA::RenderWidget* renderWidget);

    void CreateActions();
    void ApplyPosChanges();
    void OnMouseReleased(QMouseEvent* event) override;
    void OnMouseMove(QMouseEvent* event) override;
    void OnMouseDBClick(QMouseEvent* event) override;
    void OnDragEntered(QDragEnterEvent* event) override;
    void OnDragMoved(QDragMoveEvent* event) override;
    bool ProcessDragMoveEvent(QDropEvent* event);
    void OnDragLeaved(QDragLeaveEvent* event) override;
    void OnDrop(QDropEvent* event) override;
    void OnKeyPressed(QKeyEvent* event) override;

    float GetScaleFromComboboxText() const;

    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::RenderWidget* renderWidget = nullptr;

    RulerController* rulerController = nullptr;
    QPoint rootControlPos;
    QPoint canvasPos;

    QAction* selectAllAction = nullptr;
    QAction* focusNextChildAction = nullptr;
    QAction* focusPreviousChildAction = nullptr;

    EditorSystemsManager* systemsManager = nullptr;
    EditorCanvas* editorCanvas = nullptr;
    CursorInterpreter* cursorInterpreter = nullptr;

    //we can show model dialogs only when mouse released, so remember node to change text when mouse will be released
    ControlNode* nodeToChangeTextOnMouseRelease = nullptr;

    QGridLayout* gridLayout = nullptr;
    RulerWidget* horizontalRuler = nullptr;
    RulerWidget* verticalRuler = nullptr;

    GuidesController* hGuidesController = nullptr;
    GuidesController* vGuidesController = nullptr;

    FindInDocumentWidget* findInDocumentWidget = nullptr;
    QComboBox* scaleCombo = nullptr;
    QScrollBar* horizontalScrollBar = nullptr;
    QScrollBar* verticalScrollBar = nullptr;
};
