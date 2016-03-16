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


#include "PreviewWidget.h"

#include "ScrollAreaController.h"

#include <QLineEdit>
#include <QScreen>
#include <QMenu>
#include <QShortCut>
#include "UI/UIControl.h"
#include "UI/UIScreenManager.h"
#include "UI/QtModelPackageCommandExecutor.h"

#include "QtTools/DavaGLWidget/davaglwidget.h"

#include "Document.h"
#include "EditorSystems/EditorSystemsManager.h"

#include "EditorSystems/CanvasSystem.h"
#include "EditorSystems/HUDSystem.h"
#include "Ruler/RulerWidget.h"
#include "Ruler/RulerController.h"

using namespace DAVA;

namespace
{
QString ScaleStringFromReal(qreal scale)
{
    return QString("%1 %").arg(static_cast<int>(scale * 100.0f + 0.5f));
}

struct PreviewContext : WidgetContext
{
    QPoint canvasPosition;
};

struct SystemsContext : WidgetContext
{
    SelectedNodes selection;
};
}

PreviewWidget::PreviewWidget(QWidget* parent)
    : QWidget(parent)
    , davaGLWidget(new DavaGLWidget(this))
    , scrollAreaController(new ScrollAreaController(this))
    , rulerController(new RulerController(this))
{
    qRegisterMetaType<SelectedNodes>("SelectedNodes");
    percentages << 0.25f << 0.33f << 0.50f << 0.67f << 0.75f << 0.90f
                << 1.00f << 1.10f << 1.25f << 1.50f << 1.75f << 2.00f
                << 2.50f << 3.00f << 4.00f << 5.00f << 6.00f << 7.00f << 8.00f;
    setupUi(this);

    connect(rulerController, &RulerController::HorisontalRulerSettingsChanged, horizontalRuler, &RulerWidget::OnRulerSettingsChanged);
    connect(rulerController, &RulerController::VerticalRulerSettingsChanged, verticalRuler, &RulerWidget::OnRulerSettingsChanged);

    connect(rulerController, &RulerController::HorisontalRulerMarkPositionChanged, horizontalRuler, &RulerWidget::OnMarkerPositionChanged);
    connect(rulerController, &RulerController::VerticalRulerMarkPositionChanged, verticalRuler, &RulerWidget::OnMarkerPositionChanged);

    connect(scrollAreaController, &ScrollAreaController::NestedControlPositionChanged, this, &PreviewWidget::OnNestedControlPositionChanged);

    verticalRuler->SetRulerOrientation(Qt::Vertical);
    frame->layout()->addWidget(davaGLWidget);
    davaGLWidget->GetGLView()->installEventFilter(this);

    davaGLWidget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    connect(davaGLWidget, &DavaGLWidget::Initialized, this, &PreviewWidget::OnGLInitialized);
    connect(davaGLWidget, &DavaGLWidget::Resized, this, &PreviewWidget::OnGLWidgetResized);
    // Setup the Scale Combo.
    for (auto percentage : percentages)
    {
        scaleCombo->addItem(ScaleStringFromReal(percentage));
    }
    connect(scrollAreaController, &ScrollAreaController::ViewSizeChanged, this, &PreviewWidget::UpdateScrollArea);
    connect(scrollAreaController, &ScrollAreaController::CanvasSizeChanged, this, &PreviewWidget::UpdateScrollArea);
    connect(scrollAreaController, &ScrollAreaController::PositionChanged, this, &PreviewWidget::OnPositionChanged);
    connect(scrollAreaController, &ScrollAreaController::ScaleChanged, this, &PreviewWidget::OnScaleChanged);

    connect(scaleCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PreviewWidget::OnScaleByComboIndex);
    connect(scaleCombo->lineEdit(), &QLineEdit::editingFinished, this, &PreviewWidget::OnScaleByComboText);

    connect(verticalScrollBar, &QScrollBar::valueChanged, this, &PreviewWidget::OnVScrollbarMoved);
    connect(horizontalScrollBar, &QScrollBar::valueChanged, this, &PreviewWidget::OnHScrollbarMoved);

    scaleCombo->setCurrentIndex(percentages.indexOf(1.00f)); //100%
    QRegExp regEx("[0-8]?([0-9]|[0-9]){0,2}\\s?\\%?");
    scaleCombo->setValidator(new QRegExpValidator(regEx));
    scaleCombo->setInsertPolicy(QComboBox::NoInsert);
    UpdateScrollArea();
    CreateActions();
}

PreviewWidget::~PreviewWidget()
{
}

ScrollAreaController* PreviewWidget::GetScrollAreaController()
{
    return scrollAreaController;
}

RulerController* PreviewWidget::GetRulerController()
{
    return rulerController;
}

float PreviewWidget::GetScale() const
{
    // Firstly verify whether the value is already set.
    QString curTextValue = scaleCombo->currentText();
    curTextValue.remove('%');
    curTextValue.remove(' ');
    bool ok;
    float scaleValue = curTextValue.toFloat(&ok);
    DVASSERT_MSG(ok, "can not parse text to float");
    return scaleValue;
}

ControlNode* PreviewWidget::OnSelectControlByMenu(const Vector<ControlNode*>& nodesUnderPoint, const Vector2& point)
{
    QPoint globalPos = davaGLWidget->mapToGlobal(QPoint(point.x, point.y));
    QMenu menu;
    for (auto it = nodesUnderPoint.rbegin(); it != nodesUnderPoint.rend(); ++it)
    {
        ControlNode* controlNode = *it;
        QString className = QString::fromStdString(controlNode->GetControl()->GetClassName());
        QAction* action = new QAction(QString::fromStdString(controlNode->GetName()), &menu);
        action->setCheckable(true);
        menu.addAction(action);
        void* ptr = static_cast<void*>(controlNode);
        action->setData(QVariant::fromValue(ptr));
        if (selectionContainer.IsSelected(controlNode))
        {
            action->setChecked(true);
        }
    }
    QAction* selectedAction = menu.exec(globalPos);
    if (nullptr != selectedAction)
    {
        void* ptr = selectedAction->data().value<void*>();
        return static_cast<ControlNode*>(ptr);
    }
    return nullptr;
}

void PreviewWidget::CreateActions()
{
    QAction* importPackageAction = new QAction(tr("Import package"), this);
    importPackageAction->setShortcut(QKeySequence::New);
    importPackageAction->setShortcutContext(Qt::WindowShortcut);
    connect(importPackageAction, &QAction::triggered, this, &PreviewWidget::ImportRequested);
    davaGLWidget->addAction(importPackageAction);

    QAction* cutAction = new QAction(tr("Cut"), this);
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setShortcutContext(Qt::WindowShortcut);
    connect(cutAction, &QAction::triggered, this, &PreviewWidget::CutRequested);
    davaGLWidget->addAction(cutAction);

    QAction* copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setShortcutContext(Qt::WindowShortcut);
    connect(copyAction, &QAction::triggered, this, &PreviewWidget::CopyRequested);
    davaGLWidget->addAction(copyAction);

    QAction* pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setShortcutContext(Qt::WindowShortcut);
    connect(pasteAction, &QAction::triggered, this, &PreviewWidget::PasteRequested);
    davaGLWidget->addAction(pasteAction);

    QAction* deleteAction = new QAction(tr("Delete"), this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setShortcutContext(Qt::WindowShortcut); //widget shortcut is not working for davaGLWidget
    connect(deleteAction, &QAction::triggered, this, &PreviewWidget::DeleteRequested);
    davaGLWidget->addAction(deleteAction);

    selectAllAction = new QAction(tr("Select all"), this);
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    selectAllAction->setShortcutContext(Qt::WindowShortcut);
    davaGLWidget->addAction(selectAllAction);

    focusNextChildAction = new QAction(tr("Focus next child"), this);
    focusNextChildAction->setShortcut(Qt::Key_Tab);
    focusNextChildAction->setShortcutContext(Qt::WindowShortcut);
    davaGLWidget->addAction(focusNextChildAction);

    focusPreviousChildAction = new QAction(tr("Focus frevious child"), this);
    focusPreviousChildAction->setShortcut(Qt::ShiftModifier + Qt::Key_Tab);
    focusPreviousChildAction->setShortcutContext(Qt::WindowShortcut);
    davaGLWidget->addAction(focusPreviousChildAction);
}

void PreviewWidget::OnDocumentChanged(Document* arg)
{
    DVASSERT(nullptr != systemsManager);
    SaveContext();
    document = arg;
    if (document.isNull())
    {
        systemsManager->PackageNodeChanged.Emit(nullptr);
    }
    else
    {
        systemsManager->PackageNodeChanged.Emit(document->GetPackage());
        LoadContext();
    }
}

void PreviewWidget::SaveSystemsContextAndClear()
{
    if (!document.isNull())
    {
        SystemsContext* systemsContext = DynamicTypeCheck<SystemsContext*>(document->GetContext(systemsManager.get()));
        systemsContext->selection = selectionContainer.selectedNodes;
    }
    if (!selectionContainer.selectedNodes.empty())
    {
        DVASSERT(!document.isNull());
        SelectedNodes deselected = selectionContainer.selectedNodes; //this signal will remove current selectedNodes too!
        systemsManager->SelectionChanged.Emit(SelectedNodes(), deselected);
        DVASSERT(selectionContainer.selectedNodes.empty());
    }
}

void PreviewWidget::LoadSystemsContext(Document* arg)
{
    DVASSERT(arg == document.data());
    if (document.isNull())
    {
        return;
    }
    SystemsContext* context = DynamicTypeCheck<SystemsContext*>(document->GetContext(systemsManager.get()));
    if (nullptr == context)
    {
        document->SetContext(systemsManager.get(), new SystemsContext());
    }
    else
    {
        selectionContainer.selectedNodes = context->selection;
        if (!selectionContainer.selectedNodes.empty())
        {
            systemsManager->SelectionChanged.Emit(selectionContainer.selectedNodes, SelectedNodes());
        }
    }
}

void PreviewWidget::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    systemsManager->SelectionChanged.Emit(selected, deselected);
}

void PreviewWidget::OnRootControlPositionChanged(const DAVA::Vector2& pos)
{
    rootControlPos = QPoint(static_cast<int>(pos.x), static_cast<int>(pos.y));
    ApplyPosChanges();
}

void PreviewWidget::OnNestedControlPositionChanged(const QPoint& pos)
{
    canvasPos = pos;
    ApplyPosChanges();
}

void PreviewWidget::OnEmulationModeChanged(bool emulationMode)
{
    systemsManager->SetEmulationMode(emulationMode);
}

void PreviewWidget::ApplyPosChanges()
{
    QPoint viewPos = canvasPos + rootControlPos;
    rulerController->SetViewPos(-viewPos);
}

void PreviewWidget::UpdateScrollArea()
{
    QSize areaSize = scrollAreaController->GetViewSize();

    verticalScrollBar->setPageStep(areaSize.height());
    horizontalScrollBar->setPageStep(areaSize.width());

    QPoint minPos = scrollAreaController->GetMinimumPos();
    QPoint maxPos = scrollAreaController->GetMaximumPos();
    horizontalScrollBar->setRange(minPos.x(), maxPos.x());
    verticalScrollBar->setRange(minPos.y(), maxPos.y());
}

void PreviewWidget::OnPositionChanged(const QPoint& position)
{
    horizontalScrollBar->setSliderPosition(position.x());
    verticalScrollBar->setSliderPosition(position.y());
}

void PreviewWidget::OnGLInitialized()
{
    DVASSERT(nullptr == systemsManager);
    systemsManager.reset(new EditorSystemsManager());
    systemsManager->GetControlByMenu = std::bind(&PreviewWidget::OnSelectControlByMenu, this, _1, _2);
    scrollAreaController->SetNestedControl(systemsManager->GetRootControl());
    scrollAreaController->SetMovableControl(systemsManager->GetScalableControl());
    systemsManager->CanvasSizeChanged.Connect(scrollAreaController, &ScrollAreaController::UpdateCanvasContentSize);
    systemsManager->RootControlPositionChanged.Connect(this, &PreviewWidget::OnRootControlPositionChanged);
    systemsManager->SelectionChanged.Connect(this, &PreviewWidget::OnSelectionInSystemsChanged);
    systemsManager->PropertiesChanged.Connect(this, &PreviewWidget::OnPropertiesChanged);
    connect(focusNextChildAction, &QAction::triggered, [this]() { systemsManager->FocusNextChild.Emit(); });
    connect(focusPreviousChildAction, &QAction::triggered, [this]() { systemsManager->FocusPreviousChild.Emit(); });
    connect(selectAllAction, &QAction::triggered, [this]() { systemsManager->SelectAllControls.Emit(); });
}

void PreviewWidget::OnScaleChanged(qreal scale)
{
    scaleCombo->lineEdit()->setText(ScaleStringFromReal(scale));
    rulerController->SetScale(scale);
    DAVA::float32 realScale = static_cast<DAVA::float32>(scale);
    if (systemsManager)
    {
        systemsManager->GetScalableControl()->SetScale(Vector2(realScale, realScale));
    }
}

void PreviewWidget::OnScaleByComboIndex(int index)
{
    DVASSERT(index >= 0);
    float scale = static_cast<float>(percentages.at(index));
    scrollAreaController->SetScale(scale);
}

void PreviewWidget::OnScaleByComboText()
{
    float scale = GetScale();
    scrollAreaController->SetScale(scale / 100.0f);
}

void PreviewWidget::OnGLWidgetResized(int width, int height)
{
    scrollAreaController->SetViewSize(QSize(width, height));
    UpdateScrollArea();
}

void PreviewWidget::OnVScrollbarMoved(int vPosition)
{
    QPoint canvasPosition = scrollAreaController->GetPosition();
    canvasPosition.setY(vPosition);
    scrollAreaController->SetPosition(canvasPosition);
}

void PreviewWidget::OnHScrollbarMoved(int hPosition)
{
    QPoint canvasPosition = scrollAreaController->GetPosition();
    canvasPosition.setX(hPosition);
    scrollAreaController->SetPosition(canvasPosition);
}

bool PreviewWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == davaGLWidget->GetGLView())
    {
        switch (event->type())
        {
        case QEvent::Wheel:
            OnWheelEvent(DynamicTypeCheck<QWheelEvent*>(event));
            break;
        case QEvent::NativeGesture:
            OnNativeGuestureEvent(DynamicTypeCheck<QNativeGestureEvent*>(event));
            break;
        case QEvent::MouseMove:
            OnMoveEvent(DynamicTypeCheck<QMouseEvent*>(event));
            break;
        case QEvent::MouseButtonPress:
            OnPressEvent(DynamicTypeCheck<QMouseEvent*>(event));
            break;
        case QEvent::MouseButtonRelease:
            davaGLWidget->GetGLView()->unsetCursor();
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void PreviewWidget::LoadContext()
{
    PreviewContext* context = DynamicTypeCheck<PreviewContext*>(document->GetContext(this));
    if (nullptr == context)
    {
        context = new PreviewContext();
        document->SetContext(this, context);
        QPoint position(horizontalScrollBar->maximum() / 2.0f, verticalScrollBar->maximum() / 2.0f);
        scrollAreaController->SetPosition(position);
    }
    else
    {
        scrollAreaController->SetPosition(context->canvasPosition);
    }
}

void PreviewWidget::SaveContext()
{
    if (document.isNull())
    {
        return;
    }
    PreviewContext* context = DynamicTypeCheck<PreviewContext*>(document->GetContext(this));
    context->canvasPosition = scrollAreaController->GetPosition();
}

void PreviewWidget::OnWheelEvent(QWheelEvent* event)
{
    if (document.isNull())
    {
        return;
    }
//QWheelEvent::source to distinguish wheel and touchpad is implemented only in Qt 5.5
#ifdef Q_OS_WIN //under MAC OS we get this event when scrolling by two fingers on MAC touchpad
    if (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
#endif //Q_OS_WIN
    {
#ifdef Q_OS_WIN
        QPoint delta = event->angleDelta();
#else //Q_OS_MAC
        QPoint delta = event->pixelDelta();
#endif //Q_OS_WIN
        //scroll view up and down
        static const qreal wheelDelta = 0.002f;
        QPoint position = scrollAreaController->GetPosition();
        QPoint additionalPos((delta.x() * horizontalScrollBar->pageStep()) * wheelDelta,
                             (delta.y() * verticalScrollBar->pageStep()) * wheelDelta);
        scrollAreaController->SetPosition(position - additionalPos);
    }
#ifdef Q_OS_WIN
    else
    {
        //resize view
        int tickSize = 120;
        int ticksCount = event->angleDelta().y() / tickSize;
        if (ticksCount == 0)
        {
            return;
        }
        qreal scale = GetScaleFromWheelEvent(ticksCount);
        QPoint pos = event->pos();
        scrollAreaController->AdjustScale(scale, pos);
    }
#endif //Q_OS_WIN
}

void PreviewWidget::OnNativeGuestureEvent(QNativeGestureEvent* event)
{
    if (document.isNull())
    {
        return;
    }
    const qreal normalScale = 1.0f;
    const qreal expandedScale = 1.5f;
    qreal scale = scrollAreaController->GetScale();
    QPoint pos = event->pos();
    switch (event->gestureType())
    {
    case Qt::ZoomNativeGesture:
        scrollAreaController->AdjustScale(scale + event->value(), pos);
        break;
    case Qt::SmartZoomNativeGesture:
        scrollAreaController->AdjustScale((event->value() == 0.0f ? normalScale : expandedScale), pos);
        //event->value() returns 1 or 0
        break;
    default:
        break;
    }
}

void PreviewWidget::OnPressEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::MiddleButton)
    {
        davaGLWidget->GetGLView()->setCursor(Qt::ClosedHandCursor);
        lastMousePos = DynamicTypeCheck<QMouseEvent*>(event)->pos();
    }
}

void PreviewWidget::OnReleaseEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::MiddleButton)
    {
        davaGLWidget->GetGLView()->unsetCursor();
    }
}

void PreviewWidget::OnMoveEvent(QMouseEvent* event)
{
    rulerController->UpdateRulerMarkers(event->pos());
    if (event->buttons() & Qt::MiddleButton)
    {
        QPoint delta(event->pos() - lastMousePos);
        lastMousePos = event->pos();

        int horizontalScrollBarValue = horizontalScrollBar->value();
        horizontalScrollBarValue -= delta.x();
        horizontalScrollBar->setValue(horizontalScrollBarValue);

        int verticalScrollBarValue = verticalScrollBar->value();
        verticalScrollBarValue -= delta.y();
        verticalScrollBar->setValue(verticalScrollBarValue);
    }
}

qreal PreviewWidget::GetScaleFromWheelEvent(int ticksCount) const
{
    qreal scale = scrollAreaController->GetScale();
    if (ticksCount > 0)
    {
        scale = GetNextScale(scale, ticksCount);
    }
    else if (ticksCount < 0)
    {
        scale = GetPreviousScale(scale, ticksCount);
    }
    return scale;
}

qreal PreviewWidget::GetNextScale(qreal currentScale, int ticksCount) const
{
    auto iter = std::upper_bound(percentages.begin(), percentages.end(), currentScale);
    if (iter == percentages.end())
    {
        return currentScale;
    }
    ticksCount--;
    int distance = std::distance(iter, percentages.end());
    ticksCount = std::min(distance, ticksCount);
    std::advance(iter, ticksCount);
    return iter != percentages.end() ? *iter : percentages.last();
}

void PreviewWidget::OnSelectionInSystemsChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    selectionContainer.MergeSelection(selected, deselected);
    emit SelectionChanged(selected, deselected);
}

void PreviewWidget::OnPropertiesChanged(const DAVA::Vector<ChangePropertyAction>& propertyActions, size_t hash)
{
    DVASSERT(!document.isNull());
    auto commandExecutor = document->GetCommandExecutor();
    commandExecutor->ChangeProperty(propertyActions, hash);
}

qreal PreviewWidget::GetPreviousScale(qreal currentScale, int ticksCount) const
{
    auto iter = std::lower_bound(percentages.begin(), percentages.end(), currentScale);
    if (iter == percentages.end())
    {
        return currentScale;
    }
    int distance = std::distance(iter, percentages.begin());
    ticksCount = std::max(ticksCount, distance);
    std::advance(iter, ticksCount);
    return *iter;
}
