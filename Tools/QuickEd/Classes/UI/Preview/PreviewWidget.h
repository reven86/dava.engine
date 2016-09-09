#pragma once
#include "Engine/Public/Qt/RenderWidget.h"

#include "ui_PreviewWidget.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "EditorSystems/SelectionContainer.h"
#include <QWidget>
#include <QCursor>
#include <QPointer>

namespace Ui
{
class PreviewWidget;
}
class EditorSystemsManager;

class Document;
class ControlNode;
class ScrollAreaController;
class PackageBaseNode;
class RulerController;
class AbstractProperty;
class QWheelEvent;
class QNativeGestureEvent;
class QDragMoveEvent;
class ContinuousUpdater;
class QDragLeaveEvent;
class QDropEvent;

class PreviewWidget : public QWidget, public Ui::PreviewWidget, private DAVA::RenderWidget::ClientDelegate
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget* parent = nullptr);
    ~PreviewWidget();
    ScrollAreaController* GetScrollAreaController();
    RulerController* GetRulerController();
    ControlNode* OnSelectControlByMenu(const DAVA::Vector<ControlNode*>& nodes, const DAVA::Vector2& pos);

    void InjectRenderWidget(DAVA::RenderWidget* renderWidget);
    void OnWindowCreated();

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
    void OnDocumentChanged(Document* document);
    void SaveSystemsContextAndClear();
    void LoadSystemsContext(Document* document);
    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void OnRootControlPositionChanged(const DAVA::Vector2& pos);
    void OnNestedControlPositionChanged(const QPoint& pos);
    void OnEmulationModeChanged(bool emulationMode);
    void OnIncrementScale();
    void OnDecrementScale();
    void SetActualScale();

private slots:
    void OnScaleChanged(float scale);
    void OnScaleByComboIndex(int value);
    void OnScaleByComboText();

    void OnVScrollbarMoved(int position);
    void OnHScrollbarMoved(int position);

    void UpdateScrollArea();
    void OnPositionChanged(const QPoint& position);

private:
    void LoadContext();
    void SaveContext();

public:
    void CreateActions();
    void ApplyPosChanges();
    void OnWheel(QWheelEvent* event) override;
    void OnNativeGuesture(QNativeGestureEvent* event) override;
    void OnMousePressed(QMouseEvent* event) override;
    void OnMouseReleased(QMouseEvent* event) override;
    void OnMouseMove(QMouseEvent* event) override;
    void OnDragEntered(QDragEnterEvent* event) override;
    void OnDragMoved(QDragMoveEvent* event) override;
    bool ProcessDragMoveEvent(QDropEvent* event);
    void OnDragLeaved(QDragLeaveEvent* event) override;
    void OnDrop(QDropEvent* event) override;
    void OnKeyPressed(QKeyEvent* event) override;
    void OnKeyReleased(QKeyEvent* event) override;

    void OnTransformStateChanged(bool inTransformState);
    void OnPropertyChanged(ControlNode* node, AbstractProperty* property, DAVA::VariantType newValue);

    float GetScaleFromWheelEvent(int ticksCount) const;
    float GetNextScale(float currentScale, int ticksCount) const;
    float GetPreviousScale(float currentScale, int ticksCount) const;

    void OnSelectionInSystemsChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void NotifySelectionChanged();
    bool CanDragScreen() const;
    void UpdateDragScreenState();
    float GetScaleFromComboboxText() const;

    QPoint lastMousePos;
    QCursor lastCursor;
    QPointer<Document> document;
    DAVA::RenderWidget* renderWidget = nullptr;
    ScrollAreaController* scrollAreaController = nullptr;
    QList<float> percentages;

    SelectionContainer selectionContainer;
    RulerController* rulerController = nullptr;
    QPoint rootControlPos;
    QPoint canvasPos;

    QAction* selectAllAction = nullptr;
    QAction* focusNextChildAction = nullptr;
    QAction* focusPreviousChildAction = nullptr;

    std::unique_ptr<EditorSystemsManager> systemsManager;

    ContinuousUpdater* continuousUpdater = nullptr;

    SelectedNodes tmpSelected; //for continuousUpdater
    SelectedNodes tmpDeselected; //for continuousUpdater

    bool inDragScreenState = false;

    //helper members to store space button and left mouse buttons states
    bool isSpacePressed = false;
    bool isMouseLeftButtonPressed = false;
    bool isMouseMidButtonPressed = false;
};
