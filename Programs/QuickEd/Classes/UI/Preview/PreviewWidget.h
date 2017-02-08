#pragma once
#include "Engine/Qt/RenderWidget.h"

#include "EditorSystems/EditorSystemsManager.h"
#include "EditorSystems/SelectionContainer.h"
#include <QWidget>
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
class RulerController;
class EditorCanvas;
class CursorInterpreter;
class AbstractProperty;
class ContinuousUpdater;

class QGridLayout;
class RulerWidget;
class RulerController;
class QComboBox;
class QScrollBar;
class RulerController;
class QWheelEvent;
class QNativeGestureEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QMenu;

class PreviewWidget : public QFrame, private DAVA::RenderWidget::IClientDelegate
{
    Q_OBJECT
public:
    explicit PreviewWidget(DAVA::TArc::ContextAccessor* accessor, DAVA::RenderWidget* renderWidget, EditorSystemsManager* systemsManager);
    ~PreviewWidget();
    void SelectControl(const DAVA::String& path);

    void InjectRenderWidget(DAVA::RenderWidget* renderWidget);

    void OnContextWillBeChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* newOne);
    void OnContextWasChanged(DAVA::TArc::DataContext* current, DAVA::TArc::DataContext* oldOne);

    DAVA::Signal<DAVA::uint64> requestCloseTab;
    DAVA::Signal<ControlNode*> requestChangeTextInNode;

signals:
    void DeleteRequested();
    void ImportRequested();
    void CutRequested();
    void CopyRequested();
    void PasteRequested();
    void SelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void OpenPackageFile(QString path);
    void DropRequested(const QMimeData* data, Qt::DropAction action, PackageBaseNode* targetNode, DAVA::uint32 destIndex, const DAVA::Vector2* pos);

public slots:
    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
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

    void OnVScrollbarMoved(int position);
    void OnHScrollbarMoved(int position);

    //function argument used for signals compatibility
    void UpdateScrollArea(const DAVA::Vector2& size = DAVA::Vector2(0.0f, 0.0f));
    void OnPositionChanged(const DAVA::Vector2& position);
    void OnResized(DAVA::uint32 width, DAVA::uint32 height);

private:
    void InitUI(DAVA::TArc::ContextAccessor* accessor);
    void ShowMenu(const QMouseEvent* mouseEvent);
    bool AddSelectionMenuSection(QMenu* parentMenu, const QPoint& pos);
    bool CanChangeTextInControl(const ControlNode* node) const;

    void InitFromSystemsManager(EditorSystemsManager* systemsManager);

private:
    void CreateActions();
    void ApplyPosChanges();
    void OnWheel(QWheelEvent* event) override;
    void OnNativeGuesture(QNativeGestureEvent* event) override;
    void OnMouseReleased(QMouseEvent* event) override;
    void OnMouseMove(QMouseEvent* event) override;
    void OnMouseDBClick(QMouseEvent* event) override;
    void OnDragEntered(QDragEnterEvent* event) override;
    void OnDragMoved(QDragMoveEvent* event) override;
    bool ProcessDragMoveEvent(QDropEvent* event);
    void OnDragLeaved(QDragLeaveEvent* event) override;
    void OnDrop(QDropEvent* event) override;
    void OnKeyPressed(QKeyEvent* event) override;

    void OnDragStateChanged(EditorSystemsManager::eDragState dragState, EditorSystemsManager::eDragState previousState);
    void OnPropertyChanged(ControlNode* node, AbstractProperty* property, DAVA::VariantType newValue);

    float GetScaleFromWheelEvent(int ticksCount) const;
    float GetNextScale(float currentScale, int ticksCount) const;
    float GetPreviousScale(float currentScale, int ticksCount) const;

    void OnSelectionInSystemsChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void NotifySelectionChanged();
    void UpdateDragScreenState();
    float GetScaleFromComboboxText() const;

    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::RenderWidget* renderWidget = nullptr;

    QList<float> percentages;

    SelectionContainer selectionContainer;
    RulerController* rulerController = nullptr;
    QPoint rootControlPos;
    QPoint canvasPos;

    QAction* selectAllAction = nullptr;
    QAction* focusNextChildAction = nullptr;
    QAction* focusPreviousChildAction = nullptr;

    EditorSystemsManager* systemsManager = nullptr;
    EditorCanvas* editorCanvas = nullptr;
    CursorInterpreter* cursorInterpreter = nullptr;

    ContinuousUpdater* continuousUpdater = nullptr;

    SelectedNodes tmpSelected; //for continuousUpdater
    SelectedNodes tmpDeselected; //for continuousUpdater

    //we can show model dialogs only when mouse released, so remember node to change text when mouse will be released
    ControlNode* nodeToChangeTextOnMouseRelease = nullptr;

    QGridLayout* gridLayout = nullptr;
    RulerWidget* horizontalRuler = nullptr;
    RulerWidget* verticalRuler = nullptr;
    QComboBox* scaleCombo = nullptr;
    QScrollBar* horizontalScrollBar = nullptr;
    QScrollBar* verticalScrollBar = nullptr;
};
