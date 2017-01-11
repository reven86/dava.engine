#pragma once
#include "Engine/Qt/RenderWidget.h"

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
class PackageBaseNode;
class RulerController;
class EditorCanvas;
class AbstractProperty;
class QWheelEvent;
class QNativeGestureEvent;
class QDragMoveEvent;
class ContinuousUpdater;
class QDragLeaveEvent;
class QDropEvent;
class QMenu;

class PreviewWidget : public QWidget, public Ui::PreviewWidget, private DAVA::RenderWidget::IClientDelegate
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget* parent = nullptr);
    ~PreviewWidget();
    void SelectControl(const DAVA::String& path);

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
    void UpdateScrollArea(const DAVA::Vector2 &size = DAVA::Vector2(0.0f, 0.0f));
    void OnPositionChanged(const DAVA::Vector2& position);
    void OnResized(DAVA::uint32 width, DAVA::uint32 height);

private:
    void ShowMenu(const QMouseEvent* mouseEvent);
    bool AddSelectionMenuSection(QMenu* parentMenu, const QPoint& pos);
    bool CanChangeTextInControl(const ControlNode* node) const;
    void ChangeControlText(ControlNode* node);

    void LoadContext();
    void SaveContext();

private:
    void CreateActions();
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

    void OnTransformStateChanged(bool inTransformState);
    void OnPropertyChanged(ControlNode* node, AbstractProperty* property, DAVA::VariantType newValue);

    float GetScaleFromWheelEvent(int ticksCount) const;
    float GetNextScale(float currentScale, int ticksCount) const;
    float GetPreviousScale(float currentScale, int ticksCount) const;

    void OnSelectionInSystemsChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void NotifySelectionChanged();
    void UpdateDragScreenState();
    float GetScaleFromComboboxText() const;

    QPointer<Document> document;
    DAVA::RenderWidget* renderWidget = nullptr;
    
    QList<float> percentages;

    SelectionContainer selectionContainer;
    RulerController* rulerController = nullptr;
    QPoint rootControlPos;
    QPoint canvasPos;

    QAction* selectAllAction = nullptr;
    QAction* focusNextChildAction = nullptr;
    QAction* focusPreviousChildAction = nullptr;

    std::unique_ptr<EditorSystemsManager> systemsManager;
    EditorCanvas *editorCanvas = nullptr;

    ContinuousUpdater* continuousUpdater = nullptr;

    SelectedNodes tmpSelected; //for continuousUpdater
    SelectedNodes tmpDeselected; //for continuousUpdater

    //we can show model dialogs only when mouse released, so remember node to change text when mouse will be released
    ControlNode* nodeToChangeTextOnMouseRelease = nullptr;
};
