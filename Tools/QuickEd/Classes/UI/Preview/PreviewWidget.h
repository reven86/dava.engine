#ifndef __QUICKED_PREVIEW_WIDGET_H__
#define __QUICKED_PREVIEW_WIDGET_H__

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
class DavaGLWidget;
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
class QMenu;

class PreviewWidget : public QWidget, public Ui::PreviewWidget
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget* parent = nullptr);
    ~PreviewWidget();
    DavaGLWidget* GetGLWidget() const;
    ScrollAreaController* GetScrollAreaController();
    RulerController* GetRulerController();

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

    void OnGLWidgetResized(int width, int height);

    void OnVScrollbarMoved(int position);
    void OnHScrollbarMoved(int position);

    void UpdateScrollArea();
    void OnPositionChanged(const QPoint& position);
    void OnGLInitialized();

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    void ShowMenu(const QPoint& pos);
    QMenu* CreateSelectionSubMenu(QMenu* parentMenu, const QPoint& pos);
    bool CanChangeTextInControl(const ControlNode* node) const;
    void ChangeControlText(ControlNode* node);

    void LoadContext();
    void SaveContext();

    void CreateActions();
    void ApplyPosChanges();
    void OnWheelEvent(QWheelEvent* event);
    void OnNativeGuestureEvent(QNativeGestureEvent* event);
    void OnPressEvent(QMouseEvent* event);
    void OnReleaseEvent(QMouseEvent* event);
    void OnDoubleClickEvent(QMouseEvent* event);
    void OnMoveEvent(QMouseEvent* event);
    void OnDragMoveEvent(QDragMoveEvent* event);
    bool ProcessDragMoveEvent(QDropEvent* event);
    void OnDragLeaveEvent(QDragLeaveEvent* event);
    void OnDropEvent(QDropEvent* event);
    void OnKeyPressed(QKeyEvent* event);
    void OnKeyReleased(QKeyEvent* event);

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
    DavaGLWidget* davaGLWidget = nullptr;
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

inline DavaGLWidget* PreviewWidget::GetGLWidget() const
{
    return davaGLWidget;
}

#endif // __QUICKED_PREVIEW_WIDGET_H__
